cmake_minimum_required(VERSION 3.2)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS container)

pkg_check_modules(LIBCAMERA REQUIRED libcamera)





add_executable(2camerasExample example2Cameras.cpp)
add_executable(2camerasCaptureExample example2CamerasCapture.cpp)
add_executable(2camerasCalibrationExample example2CamerasCalibration.cpp)
add_executable(2camerasDisparityExample example2CamerasDisparity.cpp)



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


