# AI

## Object Detection with YOLOv11

For real-time object detection, I integrated **YOLOv11** (You Only Look Once - version 11) into my system. YOLOv11 is a fast and accurate single-stage object detector, ideal for edge devices like the Raspberry Pi or Jetson platforms due to its efficiency and speed.

I used the **pretrained standard YOLOv11 model**, which was sufficient for general-purpose detection tasks such as identifying vehicles in the camera feed.

---

## ROS2 Integration

To integrate YOLOv11 into the ROS2 ecosystem, I wrote a node in Python:

**[yolo.py](../rosPackages/sensor_fusion/python_nodes/yolo.py)**

This node performs the following:

1. **Subscribes** to a camera topic that provides live image data.
2. **Processes each frame** using the YOLOv11 detector.
3. **Extracts bounding boxes** for each detected object.
4. **Publishes** the detection results using custom ROS2 messages.

---

## Custom ROS2 Messages

To efficiently represent the detection results within ROS2, I defined two custom message types:

### 1. [BoundingBox.msg](../rosPackages/sensor_fusion_messages/msg/BoundingBox.msg)
### 2. [Detection.msg](../rosPackages/sensor_fusion_messages//msg/Detection.msg)

