FROM ros:humble-ros-base AS libcamera_builder
	WORKDIR /libcamera_build
	RUN apt update && apt install -y --no-install-recommends gnupg
	RUN apt -y upgrade
	RUN apt install -y --no-install-recommends \
		python3-pip \
		libgnutls28-dev openssl libtiff-dev pybind11-dev \
		libboost-dev \
		qtbase5-dev libqt5core5a libqt5widgets5 \
		python3-yaml python3-ply python3-jinja2
	RUN python3 -m pip install --upgrade meson ninja
	
	RUN git clone https://github.com/raspberrypi/libcamera.git && cd libcamera && git checkout e53bdf1 && cd ..
	RUN meson setup libcamera/build libcamera/ --buildtype=release -Dpipelines=rpi/vc4,rpi/pisp -Dipas=rpi/vc4,rpi/pisp -Dv4l2=true -Dgstreamer=disabled -Dtest=false -Dlc-compliance=disabled -Dcam=disabled -Dqcam=disabled -Ddocumentation=disabled -Dpycamera=enabled
	RUN ninja -C libcamera/build/

FROM ros:humble-ros-base AS kmsxx_builder
	WORKDIR /kmsxx_build
	RUN apt update
	RUN apt install -y --no-install-recommends \
		gnupg \
		python3-pip \
		libdrm-dev
	RUN python3 -m pip install --upgrade meson ninja
	
	RUN git clone https://github.com/tomba/kmsxx.git && \
	cd kmsxx && git checkout 0f18e6d && \
	meson setup build . && \
	ninja -C build/



FROM ros:humble-ros-base AS camera_ros_builder
	WORKDIR /ros_ws
	RUN apt update && apt install -y ros-humble-camera-info-manager \
			python3-pip \
			libdrm-dev \
			libgnutls28-dev openssl libtiff-dev pybind11-dev \
			libboost-dev \
			qtbase5-dev libqt5core5a libqt5widgets5 \
			python3-yaml python3-ply python3-jinja2
	
	RUN apt install ros-humble-rqt-image-view -y

	RUN python3 -m pip install --upgrade meson ninja

    # FOR THE YOLO NODE
	RUN python3 -m pip install ultralytics
	RUN python3 -m pip install "numpy<2"


	COPY --from=kmsxx_builder /kmsxx_build /kmsxx_build
	RUN cd /kmsxx_build/kmsxx/build && ninja install && cd / && rm -rf kmsxx_build
	
	COPY --from=libcamera_builder /libcamera_build/ /libcamera_build
	RUN cd /libcamera_build/libcamera/build && ninja install && cd / && rm -rf libcamera_build



	RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
	RUN cd /ros_ws
	RUN mkdir src

	RUN cd src && git clone https://github.com/christianrauch/camera_ros.git && cd camera_ros && git checkout d6a41a8143ca3a5b6904d049d07b1791dd6a547f && cd ../..
	RUN cd src && git clone https://github.com/Slamtec/sllidar_ros2.git && cd sllidar_ros2 && git checkout 34300099fadfc772965962dec837bf436706188f && cd ../..
    COPY "rosPackages/sensor_fusion" /ros_ws/src/sensor_fusion/
    COPY "rosPackages/sensor_fusion_messages" /ros_ws/src/sensor_fusion_messages/
	RUN rosdep install -y --from-paths src --ignore-src --rosdistro $ROS_DISTRO --skip-keys=libcamera
    RUN /bin/bash -c "source /opt/ros/humble/setup.bash && colcon build --packages-select sensor_fusion_messages sensor_fusion sllidar_ros2 --symlink-install"


ENTRYPOINT ["/bin/bash"]