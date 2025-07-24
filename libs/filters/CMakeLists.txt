cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(filters)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to 'Debug'")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build."
    UNLESS "CMAKE_BUILD_TYPE"
  )
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED) # Find OpenCV
find_package(cv_bridge REQUIRED) # Find cv_bridge
find_package(rclcpp_components REQUIRED)

add_library(filters SHARED src/filter.cpp) # Assuming your code is in src/filter_node.cpp
ament_target_dependencies(filters rclcpp sensor_msgs OpenCV cv_bridge rclcpp_components) # Link against the found packages

rclcpp_components_register_node(
    filters
    PLUGIN "Filter"
    EXECUTABLE filter_node
)

ament_export_targets(export_filters)
install(TARGETS filters
        EXPORT export_filters
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)
ament_package()