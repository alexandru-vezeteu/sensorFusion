FROM ros:foxy-ros-base

# Set up environment
ENV DEBIAN_FRONTEND=noninteractive

# Install ROS 2 tools (colcon, dependencies)
RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    ros-foxy-rqt \
    ros-foxy-rviz2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ros2_ws



CMD ["/bin/bash"]
