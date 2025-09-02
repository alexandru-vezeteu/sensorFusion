FROM ros:humble-ros-base
	WORKDIR /ros_ws
	RUN apt update && apt install -y ros-humble-camera-info-manager \
			python3-pip \
			libdrm-dev \
			libgnutls28-dev openssl libtiff-dev pybind11-dev \
			libboost-dev \
			qtbase5-dev libqt5core5a libqt5widgets5 \
			python3-yaml python3-ply python3-jinja2 \
            libopencv-dev
	RUN python3 -m pip install --upgrade meson ninja




	RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
	RUN cd /ros_ws
	RUN mkdir src

	COPY "rosPackages/sensor_fusion" /ros_ws/src/sensor_fusion/
    COPY "rosPackages/sensor_fusion_messages" /ros_ws/src/sensor_fusion_messages/
	COPY "camera_parameters/0.644336.yaml" /ros/_ws/stereo_calib.yaml


	RUN chmod u+x /ros_ws/src/sensor_fusion/python_nodes/yolo.py
    RUN /bin/bash -c "source /opt/ros/humble/setup.bash && colcon build --packages-select sensor_fusion_messages sensor_fusion --symlink-install"


ENTRYPOINT ["/bin/bash"]