import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from ultralytics import YOLO
import cv_bridge


MODEL_PATH = "/ros_ws/yolo11n.pt"

class Yolo(Node):

    def __init__(self):
        
        super().__init__('yolo')
        self.publisher_ = self.create_publisher(Image, 'detected_image', 10)
        self.subscription = self.create_subscription(Image, '/camera/image_raw', self.callback_received, 10)
        self.bridge = cv_bridge.CvBridge()
        try:
            self.model = YOLO(MODEL_PATH)
        except:
            exit(-1)
            


    def callback_received(self, msg):
        cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
        results = self.model.predict(source=cv_image, conf=0.8, iou=0.45, verbose=False, show=False)
        results[0].boxes
        cv_out = results[0].plot()
        out_msg = self.bridge.cv2_to_imgmsg(cv_out, encoding='bgr8')
        self.publisher_.publish(out_msg)


def main(args=None):
    rclpy.init(args=args)

    yolo_node = Yolo()

    rclpy.spin(yolo_node)

    yolo_node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()