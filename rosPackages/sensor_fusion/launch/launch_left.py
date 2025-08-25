from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.descriptions import ComposableNode
from launch_ros.actions import ComposableNodeContainer

def getContainer(suffix:str):
    return ComposableNodeContainer(
        name = f'preprocess_container_{suffix}',
        namespace='',
        executable='/opt/ros/humble/lib/rclcpp_components/component_container_mt',
        respawn=True,
        respawn_delay=0.2,
        composable_node_descriptions=[
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

def getCamera(suffix:str, camera_id:int):
    return Node(
            package='camera_ros',
            executable='camera_node',
            name=f'camera_{suffix}',
            remappings=[
                (f'/camera_{suffix}/image_raw', f'/camera_{suffix}')
            ],
            parameters=[{
                'camera': camera_id,
                'width' : 1280,
                'height' : 960,
                'role':'video',
                'format':'RGB888',
                'orientation' : 0
            }],
            respawn=True,
            respawn_delay=0.2
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

    camera_left = getCamera('left', 0)
    container_left = getContainer('left')
    yolo_left = getYolo('left')
    container_right = getContainer('right')
    camera_right = getCamera('right', 1)
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
            parameters=[{
                    'path_to_calib': '/ros_ws/stereo_calib.yaml',
                    'image_width' : 1280,
                    'image_height' : 960,
                    'matching_treshold':100000.0,
                }],
            respawn=True,
            respawn_delay=0.2
        )

    return LaunchDescription([
        container_right,
        yolo_right,
        yolo_left,
        container_left,         camera_left,
        triang,

        camera_right,

    ])