cmake_minimum_required(VERSION 3.2)
project(examples CXX)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)



set(RPLIDAR_LIB "RPLidarSDK")
set(RPLIDAR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libs/rplidar/sdk")
set(RPLIDAR_INCLUDE_DIR "${RPLIDAR_DIR}/include")
set(RPLIDAR_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../libs/rplidar/output/Linux/Release/libsl_lidar_sdk.a") # <--- Make sure this is correct for your Makefile's output

include(ExternalProject)
ExternalProject_Add(
    ${RPLIDAR_LIB}_external_build
    SOURCE_DIR "${RPLIDAR_DIR}" # This is the directory where the Makefile is located

    # IMPORTANT: For a plain Makefile, you usually don't have a 'configure' step.
    # Set this to an empty string.
    CONFIGURE_COMMAND ""

    # Correct the BUILD_COMMAND:
    # 1. Use ${CMAKE_MAKE_PROGRAM} to get the correct 'make' executable (gmake, nmake, etc.)
    # 2. Use -C to change directory to where the Makefile is.
    # 3. Use ${RPLIDAR_DIR} for the directory, not ${MY_MODULE_SOURCE_DIR} (which is undefined here).
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -C ${RPLIDAR_DIR}
    INSTALL_COMMAND ""
    # Crucial: Specify the exact path to the output file expected from the Makefile build.
    BUILD_BYPRODUCTS "${RPLIDAR_LIB_PATH}"

    # Make this external project build as part of the default 'all' target.
    ALL
)

add_library(${RPLIDAR_LIB} STATIC IMPORTED)

# IMPORTANT: The IMPORTED_LOCATION must point to the actual library file.
# You had it pointing to the directory.
set_property(TARGET ${RPLIDAR_LIB} PROPERTY IMPORTED_LOCATION "${RPLIDAR_LIB_PATH}")

set_property(TARGET ${RPLIDAR_LIB} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${RPLIDAR_INCLUDE_DIR}")

add_dependencies(
    ${RPLIDAR_LIB} # The imported target
    ${RPLIDAR_LIB}_external_build # The external project build target
)

add_executable(displayExample exampleDisplay.cpp)
add_executable(lidarExample exampleLidar.cpp)
add_executable(cameraExample exampleCamera.cpp)

target_include_directories(displayExample PUBLIC
                            ${OPENGL_INCLUDE_DIRS}  
                            ${GLUT_INCLUDE_DIRS}
                            ${GLEW_INCLUDE_DIRS}
                            ${RPLIDAR_INCLUDE_DIR}
                            )
target_link_libraries(displayExample
                    ${RPLIDAR_LIB}
                    ${GLEW_LIBRARIES}
                    ${OPENGL_LIBRARIES}
                    ${GLUT_LIBRARIES}
                    )

target_include_directories(lidarExample PUBLIC
                        ${RPLIDAR_INCLUDE_DIR}
                        )
target_link_libraries(lidarExample
                    ${RPLIDAR_LIB}
                    )       


target_include_directories(cameraExample PUBLIC
                        ${OpenCV_INCLUDE_DIRS}
                        ${LIBCAMERA_INCLUDE_DIRS}
                        )
target_link_libraries(cameraExample 
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    )