from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument





def generate_launch_description():
    yolo_left = DeclareLaunchArgument(
        'yolo_left',
        default_value='/yolo_left',
        description='Topic that supplies detections from the left camera.'
    )
    yolo_right = DeclareLaunchArgument(
        'yolo_right',
        default_value='/yolo_right',
        description='Topic that supplies detections from the right camera.'
    )
    name = DeclareLaunchArgument(
        'name',
        default_value='triangulation',
        description= 'The name of the node.'
    )
    path = DeclareLaunchArgument(
        'path_to_calib',
        default_value='/ros_ws/stereo_calib.yaml',
        description='Path to the calibration file.'
    )
    width = DeclareLaunchArgument(
        'image_width',
        default_value='1280',
    )
    height = DeclareLaunchArgument(
        'image_height',
        default_value='960'
    )
    tresh = DeclareLaunchArgument(
        'matching_treshold',
        default_value='100000.0'
    )


    triang = Node(
            package='sensor_fusion',
            namespace='',
            executable='triangulation_node',
            name=LaunchConfiguration('name'),
            remappings=[
                ('/sensor_fusion/triangulation_in_left', LaunchConfiguration('yolo_left')),
                ('/sensor_fusion/triangulation_in_right', LaunchConfiguration('yolo_right'))
            ],
            parameters=[{
                    'path_to_calib': LaunchConfiguration('path_to_calib'),
                    'image_width' : LaunchConfiguration('image_width'),
                    'image_height' : LaunchConfiguration('image_height'),
                    'matching_treshold':LaunchConfiguration('matching_treshold'),
                }],
            respawn=True,
            respawn_delay=0.2
        )
    
    return LaunchDescription([
        yolo_left,
        yolo_right,
        name,
        width,
        height,
        path,
        tresh,

        triang
    ])