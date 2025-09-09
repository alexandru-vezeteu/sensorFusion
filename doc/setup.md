# Setup

## Hardware & OS

For this project, I used a **Raspberry Pi 5** running the **minimal Raspberry Pi OS** (terminal-only version) to keep the base system lightweight. This allowed for greater control over what software gets installed.

## GUI: GNOME Desktop (Optional)

As a personal preference, I chose to install the **GNOME desktop environment** for GUI access. This isn't strictly necessary for ROS2 development, but it makes working with visual tools and debugging a bit more convenient.

To install GNOME, I used the following custom script:

[gnome.sh](../bash/gnome.sh)

This script automates the installation of the GNOME desktop on a minimal Raspberry Pi OS setup.

## Dependencies

Before diving into ROS2 or sensor integration, I created a setup script to install basic system-level dependencies required to build and run **non-ROS** examples and scripts.

[req.sh](../bash/req.sh)

This script installs essential development tools and libraries, including:

- **CMake** – Build system used for compiling C/C++ examples.
- **libcamera** – For interacting with Raspberry Pi cameras.
- Standard libraries – Useful for writing and compiling calibration and test scripts.
- OpenCV, OpenGL, Docker and everything needed to build on the host.

These tools are primarily used to:

- Verify that cameras, sensors, and system peripherals are working as expected.
- Write and test low-level examples for calibration and debugging.
- Validate hardware before integrating with ROS2.

> **Note:** This setup is independent of ROS2 and is meant to ensure that the base hardware is functional before moving on to more complex, ROS-based development.

---

## Example Projects

The environment is configured to support and run the following test and example setups:

- **Single Camera Test**: Capture and display raw image streams using `libcamera`.
- **Dual Camera Setup**: Test dual camera capture and sync without ROS.
- **LiDAR Communication**: Read raw LiDAR data using serial or USB interfaces.
- **Calibration Scripts**: Run simple C++ tools to verify sensor accuracy and field of view.

These projects help verify that each component of the system is working correctly before being wrapped into ROS2 nodes or packages.

---

## Other Used Tools

To improve the development experience and system performance on the Raspberry Pi 5, I incorporated a few additional tools and optimizations.

### Increased Swap Space

Since compiling code (especially C++ projects or ROS2 packages) can be memory-intensive, I increased the **swap size** on the Raspberry Pi. This helps prevent out-of-memory issues during builds and improves overall stability when running heavier applications.

> Increasing swap is especially useful when working with limited RAM and when compiling larger codebases directly on the Pi.

---

### Visual Studio Code (VS Code)

I used **Visual Studio Code** as my main development environment. It’s lightweight, extensible, and perfect for writing and debugging both ROS2 and non-ROS code.

I particularly benefited from the following VS Code extensions:

- **Docker Extension**  
  Allows direct interaction with Docker containers: start, stop, open shells, browse files, and manage containers easily within VS Code.

- **Remote - SSH Extension**  
  Enables seamless development over SSH. I could write and run code on the Raspberry Pi while editing files locally from my main machine.

- **Remote - Containers Extension**  
  Lets me open and work inside Docker containers as if they were native environments, with full IntelliSense, debugging, and terminal access.

These tools were lifesavers when dealing with remote development, debugging sensor scripts, and managing ROS2 containers efficiently, all without needing a full desktop experience on the Raspberry Pi itself.

---

## Summary

This initial setup phase focuses on getting the **Raspberry Pi 5** environment ready for sensor testing and development. By installing only the necessary build tools and libraries, I was able to:

- Keep the system lightweight and fast.
- Confirm that each sensor and peripheral is functioning properly.
- Build a solid foundation before introducing ROS2 into the workflow.

Note: I have also added a Dockerfile that is capable of running every example that I have mentioned.