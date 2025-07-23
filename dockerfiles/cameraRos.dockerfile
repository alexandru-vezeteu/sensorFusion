FROM ros:humble-ros-base

RUN apt update && apt install -y --no-install-recommends gnupg

RUN apt update && apt -y upgrade

RUN apt update && apt install -y --no-install-recommends \
    #meson \
	#ninja-build \
	pkg-config \
	libyaml-dev \
	python3-yaml \
	python3-ply \
	python3-jinja2 \
	libevent-dev \
	libdrm-dev \
	libcap-dev \
	python3-pip \
	python3-opencv \
    #python3-colcon-meson \
     && apt-get clean \
     && apt-get autoremove \
     && rm -rf /var/cache/apt/archives/* \
     && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --upgrade meson ninja
WORKDIR /ros_ws

# Install libcamera from source
RUN git clone https://github.com/raspberrypi/libcamera.git && cd libcamera && git checkout e53bdf1 && cd ..
RUN meson setup libcamera/build libcamera/
RUN ninja -C libcamera/build/ install


# Install kmsxx from source
RUN git clone https://github.com/tomba/kmsxx.git && cd kmsxx && git checkout 0f18e6d && cd ..
RUN meson setup kmsxx/build kmsxx/
RUN ninja -C kmsxx/build/ install 

RUN apt update && apt install ros-humble-camera-info-manager ros-humble-rqt-image-view -y
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
RUN echo "export ROS_DOMAIN_ID=5" >> ~/.bashrc
RUN echo "export ROS_LOCALHOST_ONLY=1" >> ~/.bashrc

RUN mkdir src && cd src && git clone https://github.com/christianrauch/camera_ros.git && cd camera_ros && git checkout d6a41a8 && cd ..


RUN rosdep install -y --from-paths src --ignore-src --rosdistro $ROS_DISTRO --skip-keys=libcamera
RUN colcon build --event-handlers=console_direct+



ENTRYPOINT ["/bin/bash"]