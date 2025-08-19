cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(sensorFusion)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to 'Debug'")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build."
    UNLESS "CMAKE_BUILD_TYPE"
  )
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(rclcpp_components REQUIRED)

add_library(blur_filter SHARED src/BlurFilter.cpp)
ament_target_dependencies(blur_filter rclcpp sensor_msgs OpenCV cv_bridge rclcpp_components)

rclcpp_components_register_node(
    blur_filter
    PLUGIN "BlurFilter"
    EXECUTABLE blur_filter_node
)

ament_export_targets(export_blur_filter)
install(TARGETS blur_filter
        EXPORT export_blur_filter
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)



install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()