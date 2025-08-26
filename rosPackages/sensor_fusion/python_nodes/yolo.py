#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from ultralytics import YOLO
import cv_bridge
import numpy as np
import cv2
from rclpy.qos import QoSProfile, QoSHistoryPolicy
from sensor_fusion_messages.msg import Detection, BoundingBox
from geometry_msgs.msg import Point

MODEL_PATH = "/ros_ws/yolov5nu.pt"

class Yolo(Node):

    def __init__(self):
        
        super().__init__('yolo')
        qos_profile = QoSProfile(
            depth=1,
            history=QoSHistoryPolicy.KEEP_LAST
            )
        self.publisher_ = self.create_publisher(Detection, '/sensor_fusion/yolo_out', 10)
        self.subscription = self.create_subscription(Image, '/sensor_fusion/yolo_in', self.callback_received, qos_profile)
        self.bridge = cv_bridge.CvBridge()

        
        try:
            self.model = YOLO(MODEL_PATH)
        except:
            exit(-1)
            


    def callback_received(self, msg):
        buf = np.frombuffer(msg.data, dtype=np.uint8)
        buf = buf.reshape((msg.height, msg.step))
        img_data = buf[:, :msg.width*3]
        cv_image = img_data.reshape((msg.height, msg.width, 3))


        results = self.model(cv_image)
        result = results[0]

        # COCO class index for 'car' (usually 2)
        car_class_id = 2
        boxes = []

        for box, conf, cls in zip(result.boxes.xyxy.cpu().numpy(),
                                result.boxes.conf.cpu().numpy(),
                                result.boxes.cls.cpu().numpy()):
            if int(cls) != car_class_id:
                continue  # skip non-car classes
            
            x1, y1, x2, y2 = map(float, box)
            left_up = Point(x=x1, y=y1, z=0.0)
            right_down = Point(x=x2, y=y2, z=0.0)
            # self.get_logger().info(f"{x1}, {y1}, {x2}, {y2}")

            bbox = BoundingBox()
            bbox.left_up = left_up
            bbox.right_down = right_down

            boxes.append(bbox)
        if len(boxes)!=0:
            out_msg = Detection()
            out_msg.image = msg  # Use the original image as-is
            out_msg.boxes = boxes

            self.publisher_.publish(out_msg)
            self.get_logger().info(f"Published Detection with {len(boxes)} bounding box(es)")


def main(args=None):
    rclpy.init(args=args)
    node = Yolo()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('Keyboard interrupt, shutting down.')
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()