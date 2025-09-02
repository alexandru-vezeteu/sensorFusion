cmake_minimum_required(VERSION 3.2)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS container)

pkg_check_modules(LIBCAMERA REQUIRED libcamera)





add_executable(2cameras example2Cameras.cpp)
add_executable(2capture example2CamerasCapture.cpp)
add_executable(stereocalibration example2CamerasCalibration.cpp)
add_executable(disparity example2CamerasDisparity.cpp)



target_include_directories(2cameras PUBLIC
                            ${OpenCV_INCLUDE_DIRS}
                            ${LIBCAMERA_INCLUDE_DIRS}
                            ${Boost_INCLUDE_DIRS}
                            )
target_link_libraries(2cameras
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    Boost::container
                    )

target_include_directories(2capture PUBLIC
                            ${OpenCV_INCLUDE_DIRS}
                            ${LIBCAMERA_INCLUDE_DIRS}
                            ${Boost_INCLUDE_DIRS}
                            )
target_link_libraries(2capture
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    Boost::container
                    )


target_include_directories(stereocalibration PUBLIC
                        ${OpenCV_INCLUDE_DIRS}
                        )
target_link_libraries(stereocalibration 
                    ${OpenCV_LIBRARIES}
                    )


target_include_directories(disparity PUBLIC
                    ${OpenCV_INCLUDE_DIRS}
                    ${LIBCAMERA_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    )
target_link_libraries(disparity
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    Boost::container
                    )


