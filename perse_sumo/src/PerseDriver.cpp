#include "perse_sumo/PerseDriver.hpp"

#include "rclcpp/rclcpp.hpp"
#include <cstdio>
#include <functional>
#include <iostream>
#include <ostream>
#include <rclcpp/logging.hpp>
#include <sstream>
#include <string>
#include <webots/gyro.h>
#include <webots/led.h>
#include <webots/motor.h>
#include <webots/robot.h>
#include <webots/supervisor.h>
#include <webots/touch_sensor.h>
#include <webots/vehicle/car.h>
#include <webots/vehicle/driver.h>

#define ENCODER_RESOLUTION 0.001
#define PLATE_SIZE 1.0
#define WHEELBASE 0.257

namespace perse_driver {

void PerseDriver::init(

    webots_ros2_driver::WebotsNode *node,
    std::unordered_map<std::string, std::string> &parameters) {
  this->node = node;

  wbu_car_init();
  wbu_driver_init();

  // This parameter is read when loading the URDF file
  publish_timestep = parameters.count("updateRate")
                         ? 1.0 / atof(parameters["updateRate"].c_str())
                         : 0;

  bumper = wb_robot_get_device("bumper");
  led = wb_robot_get_device("led");

  wb_touch_sensor_enable(bumper, publish_timestep * 1000);

  // WbNodeRef perse_node = wb_supervisor_node_get_from_def("PERSE");
  //
  // WbNodeRef line_sensor_node =
  //     wb_supervisor_node_get_from_proto_def(perse_node, "LINE_SENSOR");
  //
  // WbNodeRef traffic_cone_node =
  // wb_supervisor_node_get_from_def("TRAFFIC_CONE");
  //
  // wb_supervisor_node_set_visibility(traffic_cone_node, line_sensor_node,
  // false);
  //
  // if (traffic_cone_node == NULL) {
  //   std::cout << "TRAFFIC_CONE DEF not found!" << std::endl;
  // }
  // if (line_sensor_node == NULL) {
  //   std::cout << "LINE_SENSOR DEF not found!" << std::endl;
  // }

  node->declare_parameter("encoder_constant", rclcpp::PARAMETER_DOUBLE);
  encoder_constant = node->get_parameter("encoder_constant").as_double();

  cmd_vel_subscription = node->create_subscription<geometry_msgs::msg::Twist>(
      "/perse/actuator/cmd_vel", rclcpp::SensorDataQoS().reliable(),
      std::bind(&PerseDriver::cmdVelCallback, this, std::placeholders::_1));

  led_subscription = node->create_subscription<std_msgs::msg::Bool>(
      "/perse/actuator/led_rod", rclcpp::SensorDataQoS().reliable(),
      std::bind(&PerseDriver::ledCallback, this, std::placeholders::_1));

  this->encoder_pub = node->create_publisher<std_msgs::msg::Float64>(
      "/perse/sensors/encoder_fusion/distance", 10);
  std_msgs::msg::Float64 encoder_msg;
  encoder_msg.data = 0;
  encoder_pub->publish(encoder_msg);

  this->bumper_pub =
      node->create_publisher<std_msgs::msg::Bool>("/perse/sensors/bumper", 10);
  bumper_msg.data = false;
  bumper_pub->publish(bumper_msg);
}

void PerseDriver::ledCallback(const std_msgs::msg::Bool::SharedPtr msg) {
  led_msg.data = msg->data;
}

void PerseDriver::cmdVelCallback(
    const geometry_msgs::msg::Twist::SharedPtr msg) {
  cmd_vel_msg.linear = msg->linear;
  cmd_vel_msg.angular = msg->angular;
}

void PerseDriver::step() {
  if (wbu_driver_step() == -1) {
    return; // Sai se a simulação terminou
  }

  // Motor e Servo
  auto forward_speed = cmd_vel_msg.linear.x;
  auto angular_speed = cmd_vel_msg.angular.z;

  auto command_servo_left =
      angular_speed == 0.0 ? 0.0
                           : std::clamp<double>(angular_speed, -0.99, 0.99) *
                                 sgn(-forward_speed);

  wbu_driver_set_cruising_speed(forward_speed * 3.6);
  wbu_driver_set_steering_angle(command_servo_left);

  // Encoder
  double encoder_reading =
      wbu_car_get_wheel_encoder(WBU_CAR_WHEEL_REAR_LEFT) * encoder_constant;

  if (std::abs(encoder_reading - last_encoder_reading) >= ENCODER_RESOLUTION) {
    std_msgs::msg::Float64 encoder_msg;
    encoder_msg.data = std::ceil(encoder_reading * (1 / ENCODER_RESOLUTION)) /
                       (1 / ENCODER_RESOLUTION);
    encoder_pub->publish(encoder_msg);
    last_encoder_reading = encoder_msg.data;
  }

  // Bumper
  double bumper_reading = wb_touch_sensor_get_value(bumper);
  bool last_bumper_reading = bumper_msg.data;

  bumper_msg.data = bumper_reading;
  if (bumper_reading != last_bumper_reading) {
    bumper_pub->publish(bumper_msg);
  }

  // Led
  wb_led_set(led, this->led_msg.data ? 1 : 0);
}

bool PerseDriver::preStep() {
  // Update only if needed
  if (wb_robot_get_time() - last_update < publish_timestep)
    return false;
  last_update = wb_robot_get_time();
  return true;
}

} // namespace perse_driver

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(perse_driver::PerseDriver,
                       webots_ros2_driver::PluginInterface)
