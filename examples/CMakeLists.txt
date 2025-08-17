cmake_minimum_required(VERSION 3.2)



find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS container)

pkg_check_modules(LIBCAMERA REQUIRED libcamera)


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

add_executable(displayExample exampleDisplay.cpp)
add_executable(lidarExample exampleLidar.cpp)
add_executable(cameraExample exampleCamera.cpp)
add_executable(calibrationExample exampleCalibration.cpp)
add_executable(captureExample exampleCapture.cpp)
add_executable(2camerasExample example2Cameras.cpp)
add_executable(2camerasCaptureExample example2CamerasCapture.cpp)
add_executable(2camerasCalibrationExample example2CamerasCalibration.cpp)
add_executable(2camerasDisparityExample example2CamerasDisparity.cpp)

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

target_include_directories(calibrationExample PUBLIC
                        ${OpenCV_INCLUDE_DIRS}
                        )
target_link_libraries(calibrationExample 
                    ${OpenCV_LIBRARIES}
                    )


target_include_directories(captureExample PUBLIC
                    ${OpenCV_INCLUDE_DIRS}
                    ${LIBCAMERA_INCLUDE_DIRS}
                    )
target_link_libraries(captureExample 
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    )

target_include_directories(2camerasExample PUBLIC
                            ${OpenCV_INCLUDE_DIRS}
                            ${LIBCAMERA_INCLUDE_DIRS}
                            ${Boost_INCLUDE_DIRS}
                            )
target_link_libraries(2camerasExample
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    Boost::container
                    )

target_include_directories(2camerasCaptureExample PUBLIC
                            ${OpenCV_INCLUDE_DIRS}
                            ${LIBCAMERA_INCLUDE_DIRS}
                            ${Boost_INCLUDE_DIRS}
                            )
target_link_libraries(2camerasCaptureExample
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    Boost::container
                    )


target_include_directories(2camerasCalibrationExample PUBLIC
                        ${OpenCV_INCLUDE_DIRS}
                        )
target_link_libraries(2camerasCalibrationExample 
                    ${OpenCV_LIBRARIES}
                    )


target_include_directories(2camerasDisparityExample PUBLIC
                    ${OpenCV_INCLUDE_DIRS}
                    ${LIBCAMERA_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    )
target_link_libraries(2camerasDisparityExample
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    Boost::container
                    )


