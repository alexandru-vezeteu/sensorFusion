from launch import LaunchDescription
from launch_ros.actions import Node


from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import IncludeLaunchDescription
from launch.substitutions import ThisLaunchFileDir
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare




def generate_launch_description():

    

    fuser = Node(
            package='sensor_fusion',
            namespace='',
            executable='fuser_node',
            name=f'fuser',
            remappings=[
                ('/sensor_fusion/fuser_in_lidar',f'/scan'),
                ('/sensor_fusion/fuser_in_triangulation', f'/yolo_left')
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



    return LaunchDescription([
        fuser,
        lidar
    ])