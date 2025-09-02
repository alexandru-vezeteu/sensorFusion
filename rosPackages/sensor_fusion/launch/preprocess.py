from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, TextSubstitution


def generate_launch_description():

    suffix = DeclareLaunchArgument(
        'suffix',
        description='Suffix for node and topic names'
    )

    container_name = [
        TextSubstitution(text='preprocess_container_'),
        LaunchConfiguration('suffix')
    ]

    blur_filter_name = [
        TextSubstitution(text='blur_filter_'),
        LaunchConfiguration('suffix')
    ]

    noise_filter_name = [
        TextSubstitution(text='noise_filter_'),
        LaunchConfiguration('suffix')
    ]

    input = DeclareLaunchArgument(
        'input'
    )

    blur_out_topic =[
        TextSubstitution(text='/blur_out_'),
        LaunchConfiguration('suffix')
    ]

    noise_out_topic = DeclareLaunchArgument(
        'output'
    )

    container = ComposableNodeContainer(
        name=container_name,
        namespace='',
        executable='/opt/ros/humble/lib/rclcpp_components/component_container_mt',
        respawn=True,
        respawn_delay=0.2,
        composable_node_descriptions=[
            ComposableNode(
                package='sensor_fusion',
                plugin='sensorFusion::BlurFilter',
                name=blur_filter_name,
                remappings=[
                    ('/sensor_fusion/blur_filter_in', LaunchConfiguration('input')),
                    ('/sensor_fusion/blur_filter_out', blur_out_topic)
                ]
            ),
            ComposableNode(
                package='sensor_fusion',
                plugin='sensorFusion::NoiseFilter',
                name=noise_filter_name,
                remappings=[
                    ('/sensor_fusion/noise_filter_in', blur_out_topic),
                    ('/sensor_fusion/noise_filter_out', LaunchConfiguration('output'))
                ]
            )
        ]
    )

    return LaunchDescription([
        suffix, noise_out_topic, input,
        container
    ])
