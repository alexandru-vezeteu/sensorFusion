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



	COPY --from=libcamera_builder /libcamera_build/ /libcamera_build
	RUN cd /libcamera_build/libcamera/build && ninja install && cd / && rm -rf libcamera_build

	COPY --from=kmsxx_builder /kmsxx_build /kmsxx_build
	RUN cd /kmsxx_build/kmsxx/build && ninja install && cd / && rm -rf kmsxx_build
	




	RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
	RUN echo "export ROS_DOMAIN_ID=5" >> ~/.bashrc
	RUN echo "export ROS_LOCALHOST_ONLY=1" >> ~/.bashrc
	RUN cd /ros_ws
	RUN mkdir src && cd src && git clone https://github.com/christianrauch/camera_ros.git && cd camera_ros && git checkout d6a41a8 && cd ../..



	RUN rosdep install -y --from-paths src --ignore-src --rosdistro $ROS_DISTRO --skip-keys=libcamera
	RUN /bin/bash -c "source /opt/ros/humble/setup.bash && colcon build --event-handlers=console_direct+ --symlink-install"


ENTRYPOINT ["/bin/bash"]