cmake_minimum_required(VERSION 3.2)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

# find_package(OpenCV REQUIRED)
# find_package(PkgConfig REQUIRED)
# find_package(Boost REQUIRED COMPONENTS container)

# pkg_check_modules(LIBCAMERA REQUIRED libcamera)


set(RPLIDAR_LIB "RPLidarSDK")
set(RPLIDAR_DIR "${PROJECT_SOURCE_DIR}/libs/rplidar/sdk")
set(RPLIDAR_INCLUDE_DIR "${RPLIDAR_DIR}/include"; "${RPLIDAR_DIR}/src")
set(RPLIDAR_LIB_PATH "${PROJECT_SOURCE_DIR}/libs/rplidar/output/Linux/Release/libsl_lidar_sdk.a") # <--- Make sure this is correct for your Makefile's output

include(ExternalProject)
ExternalProject_Add(
    ${RPLIDAR_LIB}_external_build
    SOURCE_DIR "${RPLIDAR_DIR}"
    CONFIGURE_COMMAND ""

    
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -C ${RPLIDAR_DIR}
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${RPLIDAR_LIB_PATH}"
    ALL
)

add_library(${RPLIDAR_LIB} STATIC IMPORTED)


set_property(TARGET ${RPLIDAR_LIB} PROPERTY IMPORTED_LOCATION "${RPLIDAR_LIB_PATH}")

set_property(TARGET ${RPLIDAR_LIB} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${RPLIDAR_INCLUDE_DIR}")

add_dependencies(
    ${RPLIDAR_LIB}
    ${RPLIDAR_LIB}_external_build
)

add_executable(display exampleDisplay.cpp)
add_executable(lidar exampleLidar.cpp)


target_include_directories(display PUBLIC
                            ${OPENGL_INCLUDE_DIRS}  
                            ${GLUT_INCLUDE_DIRS}
                            ${GLEW_INCLUDE_DIRS}
                            ${RPLIDAR_INCLUDE_DIR}
                            )
target_link_libraries(display
                    ${RPLIDAR_LIB}
                    ${GLEW_LIBRARIES}
                    ${OPENGL_LIBRARIES}
                    ${GLUT_LIBRARIES}
                    )

target_include_directories(lidar PUBLIC
                        ${RPLIDAR_INCLUDE_DIR}
                        )
target_link_libraries(lidar
                    ${RPLIDAR_LIB}
                    )       
