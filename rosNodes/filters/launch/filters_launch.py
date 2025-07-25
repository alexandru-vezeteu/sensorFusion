# filters/launch/filters_launch.py

from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode

def generate_launch_description():
    """
    Launches the camera node and the filter node.
    If the camera node is also a composable node, they can be launched
    in the same container for intra-process communication.
    """

    # Option 1: Launching both nodes in a single component container (recommended for performance)
    # This assumes your 'camera_node' (the one publishing to /camera/image_raw)
    # is also built as a composable node.
    camera_and_filter_container = ComposableNodeContainer(
        name='camera_and_filter_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            # Your camera node component (replace 'camera_package' and 'CameraNodePlugin')
            # You'll need to know the package and plugin name of your camera node if it's composable.
            # Example:
            # ComposableNode(
            #     package='your_camera_package',
            #     plugin='your_camera_package::CameraNodePlugin',
            #     name='camera_node',
            #     remappings=[('/image_raw', '/camera/image_raw')] # Remap if needed
            # ),
            ComposableNode(
                package='filters', # Your package name
                plugin='Filter',   # The class name of your component
                name='inversat_filter', # The name of your node instance
                remappings=[
                    ('/camera/image_raw', '/camera/image_raw'), # Input topic
                    ('/topic/B', '/processed_image')             # Output topic (you can rename it)
                ]
            )
        ],
        output='screen',
    )

    # Option 2: Launching the filter node as a standalone executable (if you uncommented main in C++ file)
    # This would be used if you had a separate main function in your C++ code
    # and wanted to run it as a standard executable process.
    # filter_node_standalone = Node(
    #     package='filters',
    #     executable='filter_node', # The executable name from your CMakeLists.txt if not composed
    #     name='inversat_filter_standalone',
    #     remappings=[
    #         ('/camera/image_raw', '/camera/image_raw'),
    #         ('/topic/B', '/processed_image')
    #     ],
    #     output='screen'
    # )

    # If your camera node is NOT a composable node, you'd launch it as a regular Node
    # camera_node_regular = Node(
    #     package='your_camera_package', # Replace with your camera package
    #     executable='your_camera_executable', # Replace with your camera executable
    #     name='camera_node',
    #     output='screen'
    # )

    return LaunchDescription([
        camera_and_filter_container,
        # If you were using a regular camera node and a standalone filter node:
        # camera_node_regular,
        # filter_node_standalone,
    ])