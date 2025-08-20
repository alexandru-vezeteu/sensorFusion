cmake_minimum_required(VERSION 3.2)


find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBCAMERA REQUIRED libcamera)



add_executable(cameraExample exampleCamera.cpp)
add_executable(calibrationExample exampleCalibration.cpp)
add_executable(captureExample exampleCapture.cpp)
      


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
