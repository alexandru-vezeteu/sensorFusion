from launch import LaunchDescription
from launch_ros.actions import Node

from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument




 




def generate_launch_description():

    lidar = DeclareLaunchArgument(
        'lidar_topic',
        default_value='/scan',
        description='The topic on which the lidar publishes the LaserScans.'
    )
    triang = DeclareLaunchArgument(
        'triang_topic',
        default_value='/sensorFusion/triangulation_out',
        description='The topic on which the triangulation node publishes the LaserScans.'
    )
    name = DeclareLaunchArgument(
        'name',
        default_value='name'
    )
    
    fuser = Node(
            package='sensor_fusion',
            namespace='',
            executable='fuser_node',
            name=LaunchConfiguration('name'),
            remappings=[
                ('/sensor_fusion/fuser_in_lidar',LaunchConfiguration('lidar_topic')),
                ('/sensor_fusion/fuser_in_triangulation', LaunchConfiguration('triang_topic'))
            ],
            respawn=True,
            respawn_delay=0.2,
        )
    

    
    
    return LaunchDescription([
        lidar,
        triang,
        name,
        fuser,
    ])