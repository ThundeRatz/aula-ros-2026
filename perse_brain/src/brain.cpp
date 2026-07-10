#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "sensor_msgs/msg/range.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/int16.hpp"
#include "std_msgs/msg/int64.hpp"
#include "std_msgs/msg/string.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <ostream>
#include <sensor_msgs/msg/detail/range__struct.hpp>
#include <std_msgs/msg/detail/bool__struct.hpp>
#include <std_msgs/msg/detail/float64__struct.hpp>

/*****************************************
 * Private Constants
 *****************************************/

 #define MSG_BUFFER 10

double lidar_angle = 0;
double lidar_distance = 0;
bool failsafe = false;
double line_sensor = std::numeric_limits<double>::max();


/*************
 * Callbacks *
 *************/
void line_sensor_callback(sensor_msgs::msg::Range msg){
  line_sensor = msg.range;
}

void lidar_callback(sensor_msgs::msg::LaserScan msg) {
  double angle_delta = (msg.angle_max - msg.angle_min) / msg.ranges.size();

  auto N = msg.ranges.size();

  double x_avg = 0;
  double y_avg = 0;
  double current_lidar_distance = 0;

  int N_valid = 0;
  for (size_t i = 0; i < N; i++) {
    if (msg.ranges[i] < msg.range_max && msg.ranges[i] > msg.range_min) {
      current_lidar_distance += msg.ranges[i];
      double angle = i * angle_delta - msg.angle_min;
      x_avg += std::cos(angle) / N;
      y_avg += std::sin(angle) / N;
      N_valid += 1;
    }
  }
  current_lidar_distance /= N_valid;

  double current_lidar_angle = std::atan2(y_avg, x_avg);
  double alpha = 0.8;

  lidar_angle = alpha * current_lidar_angle + (1 - alpha) * lidar_angle;
  lidar_distance =
      alpha * current_lidar_distance + (1 - alpha) * lidar_distance;
}

/*****************
 * Main Function *
 *****************/

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::Node::SharedPtr ros_node = rclcpp::Node::make_shared("perse_brain");

  auto failsafe_sub = ros_node->create_subscription<std_msgs::msg::Bool>(
      "/perse/sensors/failsafe", 1, &failsafe_callback);
  auto line_sensor_sub = ros_node->create_subscription<sensor_msgs::msg::Range>(
      "/perse/sensors/line_sensor", 1, &line_sensor_callback);
  auto lidar_sub = ros_node->create_subscription<sensor_msgs::msg::LaserScan>(
      "/perse/sensors/lidar", 1, &lidar_callback);
  auto cmd_vel_pub = ros_node->create_publisher<geometry_msgs::msg::Twist>(
      "/perse/actuator/cmd_vel", 1);

  geometry_msgs::msg::Twist twist_msg;

  // Declare parameters
  ros_node->declare_parameter("kp", rclcpp::PARAMETER_DOUBLE);
  ros_node->declare_parameter("line_sensor_threshold",
                              rclcpp::PARAMETER_DOUBLE);
  ros_node->declare_parameter("lidar_distance_threshold",
                              rclcpp::PARAMETER_DOUBLE);
  ros_node->declare_parameter("max_velocity", rclcpp::PARAMETER_DOUBLE);

  // get parameters
  double kp = ros_node->get_parameter("kp").as_double();
  double line_sensor_threshold =
      ros_node->get_parameter("line_sensor_threshold").as_double();
  double lidar_distance_threshold =
      ros_node->get_parameter("lidar_distance_threshold").as_double();
  double max_velocity = ros_node->get_parameter("max_velocity").as_double();

  rclcpp::Rate loop_rate(100);
  
  while (rclcpp::ok()) {
     double angle_err = kp * (0 - lidar_angle);

    std::cout << "angle: " << lidar_angle << "; distance: " << lidar_distance
              << std::endl;

    twist_msg.angular.z = -angle_err;
    twist_msg.linear.x = max_velocity / 2;

    if (lidar_distance < lidar_distance_threshold) {
      twist_msg.angular.z = 0;
      twist_msg.linear.x = max_velocity;
    }

    if (line_sensor < line_sensor_threshold) {
      twist_msg.linear.x = 0;
    }

    cmd_vel_pub->publish(twist_msg);
    rclcpp::spin_some(ros_node);
    loop_rate.sleep();
  }
}
