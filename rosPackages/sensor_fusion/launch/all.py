from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.descriptions import ComposableNode
from launch_ros.actions import ComposableNodeContainer


from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import IncludeLaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare

 




def generate_launch_description():
    camera_left = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'camera.py'
        ])
    ]),
    launch_arguments={
        'id' : '0',
        'output':'/camera_left',
        'name': 'camera_left'
    }.items())

    container_left = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'preprocess.py'
        ])
    ]),
    launch_arguments={
        'suffix':'left',
        'input' : '/camera_left',
        'output':'/noise_out_left'
    }.items())

    yolo_left = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'yolo.py'
        ])
    ]),
    launch_arguments={
        'input' : '/camera_left',
        'output':'/yolo_left_output',
        'image_output':'/yolo_left_image_output'
    }.items())



    

    camera_right = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'camera.py'
        ])
    ]),
    launch_arguments={
        'id' : '1',
        'output':'/camera_right',
        'name': 'camera_right'
    }.items())

    container_right = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'preprocess.py'
        ])
    ]),
    launch_arguments={
        'suffix':'right',
        'input' : '/camera_right',
        'output':'/noise_out_right'
    }.items())

    yolo_right = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'yolo.py'
        ])
    ]),
    launch_arguments={
        'input' : '/camera_right',
        'output':'/yolo_right_output',
        'image_output':'/yolo_right_image_output'
    }.items())


    
    lidar = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'lidar.py'
        ])
    ]))

    triang = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'triang.py'
        ])
    ]),
    launch_arguments={
        'yolo_left' : '/yolo_left_output',
        'yolo_right':'/yolo_right_output',
    }.items())
    

    fuser = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        PathJoinSubstitution([
            FindPackageShare('sensor_fusion'),
            'launch',
            'fuser.py'
        ])
    ]),
    launch_arguments={
        'yolo_left' : '/yolo_left_output',
        'yolo_right':'/yolo_right_output',
    }.items())
    
    
    
    return LaunchDescription([
        camera_left, yolo_left,
        camera_right, yolo_right,
        lidar, triang, fuser
    ])