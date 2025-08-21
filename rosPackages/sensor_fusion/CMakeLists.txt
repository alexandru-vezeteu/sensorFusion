cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(sensor_fusion)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to 'Debug'")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build."
    UNLESS "CMAKE_BUILD_TYPE"
  )
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

find_package(rclpy REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_fusion_messages REQUIRED) 
find_package(message_filters REQUIRED)




add_library(blur_filter SHARED src/BlurFilter.cpp)
ament_target_dependencies(	blur_filter 
							rclcpp 
							sensor_msgs 
							OpenCV 
							cv_bridge 
							rclcpp_components
						)
rclcpp_components_register_node(
    blur_filter
    PLUGIN "sensorFusion::BlurFilter"
    EXECUTABLE blur_filter_node
)
ament_export_targets(export_blur_filter)
install(TARGETS blur_filter
        EXPORT export_blur_filter
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)




add_library(noise_filter SHARED src/NoiseFilter.cpp)
ament_target_dependencies(noise_filter
                          rclcpp 
						  sensor_msgs 
						  OpenCV 
						  cv_bridge 
						  rclcpp_components 
						  )
rclcpp_components_register_node(
    noise_filter
    PLUGIN "sensorFusion::NoiseFilter"
    EXECUTABLE noise_filter_node
)
ament_export_targets(export_noise_filter)
install(TARGETS noise_filter
        EXPORT export_noise_filter
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)


add_library(triangulation SHARED src/Triangulation.cpp)
ament_target_dependencies(
  triangulation
  rclcpp 
  sensor_msgs 
  OpenCV 
  cv_bridge 
  rclcpp_components 
  sensor_fusion_messages
  message_filters
)
rclcpp_components_register_node(
    triangulation
    PLUGIN "sensorFusion::Triangulation"
    EXECUTABLE triangulation_node
)
ament_export_targets(export_triangulation)
install(TARGETS triangulation
        EXPORT export_triangulation
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)




add_library(fuser SHARED src/Fuser.cpp)
ament_target_dependencies(
  fuser
  rclcpp 
  sensor_msgs 
  rclcpp_components 
  message_filters
)
rclcpp_components_register_node(
    fuser
    PLUGIN "sensorFusion::Fuser"
    EXECUTABLE fuser_node
)
ament_export_targets(export_fuser)
install(TARGETS fuser
        EXPORT export_fuser
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)










install(PROGRAMS
  python_nodes/yolo.py
  DESTINATION lib/${PROJECT_NAME}
)


install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()