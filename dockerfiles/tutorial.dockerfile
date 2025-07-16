FROM ros:humble-ros-base

# Set up environment
ENV DEBIAN_FRONTEND=noninteractive

# Install ROS 2 tools (colcon, dependencies)
RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    ros-humble-rqt \
    ros-humble-rviz2 \
    vim \
    ros-humble-turtlesim \
    ~nros-humble-rqt* \
    ros-humble-rqt-common-plugins \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ros2_ws

RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc && \
    echo "export ROS_DOMAIN_ID=5" >> ~/.bashrc && \
    echo "export ROS_LOCALHOST_ONLY=1" >> ~/.bashrc


ENTRYPOINT ["/bin/bash"]
