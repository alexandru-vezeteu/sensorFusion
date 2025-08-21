from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.descriptions import ComposableNode
from launch_ros.actions import ComposableNodeContainer

def getContainer(suffix:str, camera_id:int):
    return ComposableNodeContainer(
        name = f'preprocess_container_{suffix}',
        namespace='',
        executable='/opt/ros/humble/lib/rclcpp_components/component_container_mt',
        respawn=True,
        respawn_delay=0.2,
        composable_node_descriptions=[
            ComposableNode(
                package='camera_ros',
                plugin='camera::CameraNode',
                name=f'camera_{suffix}',
                remappings=[
                    (f'/camera_{suffix}/image_raw', f'/camera_{suffix}')
                ],
                parameters=[{
                    'camera': camera_id,
                    'width' : 1920,
                    'height' : 1080,
                    'role':'video',
                    'format':'RGB888',
                    'orientation' : 0
                }]
            ),
            ComposableNode(
                package='sensor_fusion',
                plugin='sensorFusion::BlurFilter', 
                name=f'blur_filter_{suffix}',
                remappings=[
                    ('/sensor_fusion/blur_filter_in', f'/camera_{suffix}'),
                    ('/sensor_fusion/blur_filter_out', f'/blur_out_{suffix}')
                ]
            ),
            ComposableNode(
                package='sensor_fusion',
                plugin='sensorFusion::NoiseFilter', 
                name=f'noise_filter_{suffix}',
                remappings=[
                    ('/sensor_fusion/noise_filter_in', f'/blur_out_{suffix}'),
                    ('/sensor_fusion/noise_filter_out', f'/noise_out_{suffix}')
                ]
            ), 
        ]
    )

def getYolo(suffix:str):
    return Node(
            package='sensor_fusion',
            namespace='',
            executable='yolo.py',
            name=f'yolo_{suffix}',
            remappings=[
                ('/sensor_fusion/yolo_in',f'/noise_out_{suffix}'),
                ('/sensor_fusion/yolo_out', f'/yolo_{suffix}')
            ],
            respawn=True,
            respawn_delay=0.2
        )

def generate_launch_description():


    container_left = getContainer('left', 0)
    yolo_left = getYolo('left')
    container_right = getContainer('right', 1)
    yolo_right = getYolo('right')

    triang = Node(
            package='sensor_fusion',
            namespace='',
            executable='triangulation_node',
            name=f'triangulation',
            remappings=[
                ('/sensor_fusion/triangulation_in_left',f'/yolo_right'),
                ('/sensor_fusion/triangulation_in_right', f'/yolo_left')
            ],
            respawn=True,
            respawn_delay=0.2
        )

    return LaunchDescription([
        container_right,
        yolo_right,
        container_left, 
        yolo_left,
        triang
    ])