# Camera

## Hardware Setup

I used two **Raspberry Pi CSI cameras** connected to the Pi via the ribbon connectors. Initially, I experimented with one wide-angle and one standard field-of-view camera. However, this setup proved unsuitable for stereo vision:

- The **difference in focal lengths and lens distortions** between the two cameras led to poor results during stereo calibration and depth estimation.
- Switching to **two identical CSI cameras** provided much better consistency in terms of lens parameters and field of view, resulting in more reliable stereo image matching and calibration.

## Camera Interface: libcamera

To interact with the cameras, I used **libcamera**, the official camera stack for Raspberry Pi. It allows low-level control of camera parameters and provides a set of utilities and APIs for image capture and streaming.

> **Recommendation**: Install `libcamera` from the official Raspberry Pi OS repositories. Avoid building it from source unless absolutely necessary, as the build process is long and can lead to compatibility issues on the Pi.

That said, I **did build libcamera from source inside Docker containers** for ROS2-specific use cases to maintain environment control and ensure compatibility with containerized builds.

---

## Single Camera Examples

These are simple tools developed in C++ using OpenCV and libcamera to test camera functionality **outside of ROS2**. They are useful for validating the hardware setup before moving on to more complex tasks like stereo processing.

- **[1camera](../examples/camera/exampleCamera.cpp)**  
  Displays a live preview of a single camera feed in a window. Press `q` or `Q` to quit.

- **[capture](../examples/camera/exampleCapture.cpp)**  
  Same as the preview example, but also supports saving frames:
  - Press `c` to save the current frame.
  - Images are saved in a `pics/` folder created automatically in the working directory.

- **[calibration](../examples/camera/exampleCalibration.cpp)**  
  Takes a folder of chessboard images and performs **intrinsic camera calibration** using OpenCV. It outputs the camera matrix and distortion coefficients, which are essential for undistorting images and further stereo calibration.

---

## Why Camera Calibration Matters

Camera calibration is essential when working with any computer vision pipeline involving:

- Stereo vision
- Depth estimation
- 3D reconstruction
- Augmented reality
- Lens undistortion

### Calibration Process Overview

Calibration computes the camera’s **intrinsic parameters** (like focal length, principal point, distortion) and, for stereo setups, also **extrinsic parameters** (rotation and translation between cameras).

#### Intrinsic Calibration Workflow

1. Capture multiple images of a **calibration pattern** (usually a chessboard) from different angles.
2. Detect inner corners of the chessboard using OpenCV.
3. Run `cv::calibrateCamera()` to estimate:
   - Camera matrix (focal length, optical center)
   - Distortion coefficients

#### Tips for Good Calibration

- Ensure the chessboard fills the frame from various angles.
- Use 20–30 good-quality images.
- Avoid motion blur and poor lighting.
- Cover different parts of the frame (edges and center).
- Avoid using compressed or low-resolution images.



---

## Two-Camera (Stereo) Examples

Once each camera is calibrated individually, stereo calibration and depth estimation can be performed. Below are the examples used to handle two-camera setups:

- **[2cameras](../examples/2cameras/example2Cameras.cpp)**  
  Streams and displays both camera feeds side by side to verify proper synchronization and alignment.

- **[2capture](../examples/2cameras/example2CamerasCapture.cpp)**  
  Captures images from both cameras simultaneously:
  - Left images saved in `pics/left/`
  - Right images saved in `pics/right/`

- **[stereocalibration](../examples/2cameras/example2CamerasCalibration.cpp)**  
  Takes the individual intrinsic calibration results from each camera and performs **stereo calibration** using OpenCV’s `stereoCalibrate()` function.
  - Computes rotation (`R`) and translation (`T`) between the two cameras.
  - Also outputs the essential and fundamental matrices.
  - Results are stored in a stereo calibration file for later use.

- **[disparity](../examples/2cameras/example2CamerasDisparity.cpp)**  
  Computes and visualizes the **disparity map** using OpenCV’s block-matching algorithm.
  - Includes tunable parameters like block size, minimum/maximum disparity, and uniqueness ratio.
  - These parameters are critical for improving the quality of the depth map and can be configured based on the stereo setup.

> Disparity maps are sensitive to noise, lighting, camera misalignment, and calibration errors. Proper stereo calibration is key to generating usable depth maps.

> **RMS Error**: The reprojection root mean square (RMS) error reported by OpenCV after calibration should ideally be **under 0.5 pixels**. Anything above 1.0 suggests poor calibration. The best I got is [0.644336](../camera_parameters/0.644336.yaml).

---

## Triangulation: Estimating 3D Points

Once stereo calibration is complete and disparity maps are reliable, it's possible to **triangulate 3D points** from corresponding pixels in the two images.

OpenCV provides the `triangulatePoints()` function, which requires:

- The **projection matrices** of both cameras
- Matched 2D points from the left and right images

This function returns 3D coordinates in homogeneous form, which can be used to:

- Generate point clouds
- Measure distances to objects
- Feed data into 3D reconstruction pipelines

> Accurate triangulation depends on good stereo calibration, clean corner detection, and sub-pixel refinement of feature matching.

---

## Summary

Working with cameras on the Raspberry Pi, especially for stereo vision, requires careful hardware selection, proper calibration, and validation through small example programs.

- Using **identical CSI cameras** avoids mismatches that complicate stereo calibration.
- **libcamera** is the recommended interface, and works well when installed from official repositories.
- Calibration is a critical first step before any stereo or 3D processing.
- Good calibration images are hard to capture (lighting, angle, and coverage matter).
- **Stereo calibration** gives you the geometric relationship between cameras, which enables **disparity estimation** and **3D triangulation**.

These tools and processes lay the foundation for integrating stereo vision into more complex systems like ROS2, robotics, or computer vision applications.
