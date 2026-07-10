import os
import launch
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from webots_ros2_driver.webots_launcher import WebotsLauncher
from webots_ros2_driver.webots_controller import WebotsController
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration


def launch_setup(context, *args, **kwargs):
    package_dir = get_package_share_directory("perse_sumo")

    robot_description_path = os.path.join(package_dir, "resource", "perse.urdf")
    webots = WebotsLauncher(world=os.path.join(package_dir, "worlds", "world.wbt"))

    perse_driver = WebotsController(
        robot_name="perse",
        parameters=[
            {
                "robot_description": robot_description_path,
                "encoder_constant": 1 / 14,
            }
        ],
        respawn=True,
    )

    return [
        webots,
        perse_driver,
        launch.actions.RegisterEventHandler(
            event_handler=launch.event_handlers.OnProcessExit(
                target_action=webots,
                on_exit=[launch.actions.EmitEvent(event=launch.events.Shutdown())],
            )
        ),
    ]


def generate_launch_description():
    declared_arguments = []
    return LaunchDescription(
        declared_arguments + [OpaqueFunction(function=launch_setup)]
    )
