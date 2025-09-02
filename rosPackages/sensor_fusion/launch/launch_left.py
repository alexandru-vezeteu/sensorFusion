from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.descriptions import ComposableNode
from launch_ros.actions import ComposableNodeContainer


from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import IncludeLaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare

 




def generate_launch_description():

    triang = Node(
            package='sensor_fusion',
            namespace='',
            executable='triangulation_node',
            name=f'triangulation',
            remappings=[
                ('/sensor_fusion/triangulation_in_left',f'/yolo_left'),
                ('/sensor_fusion/triangulation_in_right', f'/yolo_right')
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
    
    fuser = Node(
            package='sensor_fusion',
            namespace='',
            executable='fuser_node',
            name=f'fuser',
            remappings=[
                ('/sensor_fusion/fuser_in_lidar',f'/scan'),
                ('/sensor_fusion/fuser_in_triangulation', f'/sensorFusion/triangulation_out')
            ],
            # parameters=[{
            #         'path_to_calib': '/ros_ws/stereo_calib.yaml',
            #         'image_width' : 1280,
            #         'image_height' : 960,
            #         'matching_treshold':100000.0,
            #     }],
            respawn=True,
            respawn_delay=0.2
        )
    
    lidar = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sllidar_ros2'),
            'launch',
            'sllidar_a1_launch.py'
        ])
    ]))
    
    suffix ='left'
    yolo_left = Node(
            package='sensor_fusion',
            namespace='',
            executable='yolo.py',
            name=f'yolo_{suffix}',
            remappings=[
                ('/sensor_fusion/yolo_in',f'/noise_out_{suffix}'),
                ('/sensor_fusion/yolo_out', f'/yolo_{suffix}'),
                ('/sensor_fusion/yolo_image_out', f'/yolo_image_{suffix}'),
            ],
            respawn=True,
            respawn_delay=0.2
        )
    
    suffix ='right'
    yolo_right = Node(
            package='sensor_fusion',
            namespace='',
            executable='yolo.py',
            name=f'yolo_{suffix}',
            remappings=[
                ('/sensor_fusion/yolo_in',f'/noise_out_{suffix}'),
                ('/sensor_fusion/yolo_out', f'/yolo_{suffix}'),
                ('/sensor_fusion/yolo_image_out', f'/yolo_image_{suffix}'),
            ],
            respawn=True,
            respawn_delay=0.2
        )

    suffix = 'left'
    camera_id = 0
    camera_left = Node(
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
    
    suffix = 'right'
    camera_id = 1
    camera_right = Node(
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
    
    suffix = 'right'
    container_right = ComposableNodeContainer(
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
    
    suffix = 'left'
    container_left = ComposableNodeContainer(
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
    
    
    return LaunchDescription([
        camera_left,
        camera_right,
        container_left,
        container_right,  
        yolo_left, 
        yolo_right,      
        triang,
        lidar,
        fuser,
    ])