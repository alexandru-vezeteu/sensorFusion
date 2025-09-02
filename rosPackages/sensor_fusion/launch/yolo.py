from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument


 




def generate_launch_description():


    name = DeclareLaunchArgument(
        'name',
        default_value='yolo',
        description= 'The name of the node.'
    )
    
    input = DeclareLaunchArgument(
        'input',
    )
    out = DeclareLaunchArgument(
        'output'
    )
    image_out = DeclareLaunchArgument(
        'image_output'
    )
    

    yolo = Node(
            package='sensor_fusion',
            namespace='',
            executable='yolo.py',
            name=LaunchConfiguration('name'),
            remappings=[
                ('/sensor_fusion/yolo_in',LaunchConfiguration('input')),
                ('/sensor_fusion/yolo_out', LaunchConfiguration('output')),
                ('/sensor_fusion/yolo_image_out', LaunchConfiguration('image_output')),
            ],
            respawn=True,
            respawn_delay=0.2
        )


    
    
    return LaunchDescription([
        name,  input, out, image_out,
        yolo, 

    ])