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

double lidar_angle = 0;
double lidar_distance = 0;
bool failsafe = false;
double line_sensor = std::numeric_limits<double>::max();


/*************
 * Callbacks *
 *************/
void line_sensor_callback(sensor_msgs::msg::Range msg); // Precisa ser implementado

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
  rclcpp::Rate loop_rate(100);

  /**********************************
   * Implemente os subscribers aqui *
   **********************************/
  // auto nome_sub = ros_node->create_subscription<tipo>("tópico", 1, &callback)

  /******************************
   * Declare os parâmetros aqui *
   ******************************/
  // ros_node->declare_parameter("nome_do_parametro", rclcpp::TIPO_DO_PARAMETRO);
  
  /******************************
   * Obtenha os parâmetros aqui *
   ******************************/
  // auto parametro = ros_node->get_parameter("nome_do_parametro").as_tipo();
  
  while (rclcpp::ok()) {
    /**************************************
     * Implemente a lógica principal aqui *
     **************************************/

    rclcpp::spin_some(ros_node);
    loop_rate.sleep();
  }
}
