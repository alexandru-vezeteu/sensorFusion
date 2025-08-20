from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.descriptions import ComposableNode
from launch_ros.actions import ComposableNodeContainer

def generate_launch_description():
    container = ComposableNodeContainer(
        name = 'preprocess_container',
        namespace='',
        executable='/opt/ros/humble/lib/rclcpp_components/component_container_mt',
        composable_node_descriptions=[
            ComposableNode(
                package='sensor_fusion',
                plugin='sensorFusion::BlurFilter', 
                name='blur_filter',
                remappings=[
                    ('camera', '/camera_1_3')
                ]
            ),
            
            ComposableNode(
                package='camera_ros',
                plugin='camera::CameraNode',
                name='camera_right',
                # remappings=[
                #     ('/camera_right/image_raw', '/camera_3')
                # ],
                parameters=[{
                    'camera': 0,
                    'width' : 1640,
                    'height' : 1232,
                    'role':'video',
                    'format':'RGB888'
                }]
            ),
        ]
    )


    return LaunchDescription([
        container
    ])