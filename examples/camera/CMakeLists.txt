cmake_minimum_required(VERSION 3.2)


find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBCAMERA REQUIRED libcamera)



add_executable(1camera exampleCamera.cpp)
add_executable(calibration exampleCalibration.cpp)
add_executable(capture exampleCapture.cpp)
      


target_include_directories(1camera PUBLIC
                        ${OpenCV_INCLUDE_DIRS}
                        ${LIBCAMERA_INCLUDE_DIRS}
                        )
target_link_libraries(1camera
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    )

target_include_directories(calibration PUBLIC
                        ${OpenCV_INCLUDE_DIRS}
                        )
target_link_libraries(calibration
                    ${OpenCV_LIBRARIES}
                    )


target_include_directories(capture PUBLIC
                    ${OpenCV_INCLUDE_DIRS}
                    ${LIBCAMERA_INCLUDE_DIRS}
                    )
target_link_libraries(capture
                    ${OpenCV_LIBRARIES} 
                    ${LIBCAMERA_LIBRARIES}
                    )
