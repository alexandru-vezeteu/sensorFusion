from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument
from launch.substitutions import TextSubstitution




def generate_launch_description():

    
    name = DeclareLaunchArgument(
        'name'
    )
    id = DeclareLaunchArgument(
        'id'
    )
    out = DeclareLaunchArgument(
        'output'
    )
    width = DeclareLaunchArgument(
        'width',
        default_value='1280'
    )
    height = DeclareLaunchArgument(
        'height',
        default_value='960'
    )

    source_topic = [
        TextSubstitution(text='/'),
        LaunchConfiguration('name'),
        TextSubstitution(text='/image_raw')
    ]


    camera = Node(
            package='camera_ros',
            executable='camera_node',
            name=LaunchConfiguration('name'),
            remappings=[
                (source_topic, LaunchConfiguration('output'))
            ],
            parameters=[{
                'camera': LaunchConfiguration('id'),
                'width' : LaunchConfiguration('width'),
                'height' : LaunchConfiguration('height'),
                'role':'video',
                'format':'RGB888',
                'orientation' : 0
            }],
            respawn=True,
            respawn_delay=0.2
        )
    
 
    
    
    return LaunchDescription([
        name, id, out, width, height, 
        camera
    ])