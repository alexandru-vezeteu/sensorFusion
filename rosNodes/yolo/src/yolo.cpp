#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <cv_bridge/cv_bridge.h>
#include <rclcpp_components/register_node_macro.hpp>
const std::vector<std::string> classNames = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

const float CONFIDENCE_THRESHOLD = 0.8f;
const float NMS_THRESHOLD = 0.45f;

class Yolo : public rclcpp::Node
{
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscriber_;
    cv::dnn::Net net;

    std::vector<int> indices_;
    std::vector<cv::Rect>boxes_;
    std::vector<int> classIds_;
    std::vector<float> confidences_;
public:

    Yolo(const rclcpp::NodeOptions & options)
    : rclcpp::Node("yolo", options)
    {
        std::string modelPath = "/ros_ws/yolov10n.onnx"; 

        try 
        {
            net = cv::dnn::readNet(modelPath);
            if (net.empty()) {
                std::cerr << "Error: Failed to load DNN model from " << modelPath << std::endl;
                exit(-1);
            }
            net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
            std::cout << "Successfully loaded YOLO model: " << modelPath << std::endl;
        }
        catch (const cv::Exception& e)
        {
            std::cerr << "Error loading model: " << e.what() << std::endl;
            std::cerr << "Please ensure '" << modelPath << "' exists and is a valid ONNX model." << std::endl;
            exit(-1);
        }
        
        publisher_ = this->create_publisher<sensor_msgs::msg::Image>("/detected", 10);

        subscriber_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/image_raw",
            10,
            std::bind(&Yolo::topic_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Filter node has been initialized!");
        RCLCPP_INFO(this->get_logger(), "Subscribing to /topic/A and publishing to /topic/B");
    }

    

private:



    void detectObjects(cv::Mat& frame)
    {
    
        cv::Mat blob;
        cv::dnn::blobFromImage(frame, blob, 1.0/255.0, cv::Size(640, 640), cv::Scalar(), true, false, CV_32F);

        net.setInput(blob);

        std::vector<cv::Mat> outputs;
        net.forward(outputs, net.getUnconnectedOutLayersNames());

        cv::Mat detection_output = outputs[0].reshape(1, outputs[0].size[1]);
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
    
        for (int i = 0; i < detection_output.rows; ++i)
        {
            float* data = (float*)detection_output.row(i).data; // Get pointer to the current row

            float x1 = data[0];
            float y1 = data[1];
            float x2 = data[2];
            float y2 = data[3];
            float confidence = data[4];
            int classId = static_cast<int>(data[5]);
            

            if (confidence >= CONFIDENCE_THRESHOLD) {
                float x_factor = static_cast<float>(frame.cols) / 640.0f;
                float y_factor = static_cast<float>(frame.rows) / 640.0f;

                int left = static_cast<int>(x1 * x_factor);
                int top = static_cast<int>(y1 * y_factor);
                int right = static_cast<int>(x2 * x_factor);
                int bottom = static_cast<int>(y2 * y_factor);

                // Create the bounding box rectangle
                int width = std::max(0, right - left);
                int height = std::max(0, bottom - top);

                // Or, more correctly, ensure x1 is always the minimum x, and y1 the minimum y
                int actual_left = std::min(left, right);
                int actual_top = std::min(top, bottom);
                int actual_width = std::abs(right - left);
                int actual_height = std::abs(bottom - top);

                boxes.push_back(cv::Rect(actual_left, actual_top, actual_width, actual_height));
                            confidences.push_back(confidence);
                            classIds.push_back(classId);
                }
            }

            // Apply Non-Maximum Suppression to remove redundant overlapping boxes
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);
        boxes_ = boxes;
        indices_ = indices;
        confidences_ = confidences;
        classIds_ = classIds;
    }

    void addMarkers(cv::Mat& frame)
    {
        for (int idx : indices_) {
        // You can print the index here to confirm detections are being processed
        // std::cout << "Detected idx: " << idx << std::endl;
            cv::Rect box = boxes_[idx];
            int classId = classIds_[idx];
            float confidence = confidences_[idx];

            // Ensure classId is within bounds
            if (classId < 0 || classId >= classNames.size()) {
                std::cerr << "Warning: Invalid class ID " << classId << " detected." << std::endl;
                continue;
            }

            // Draw rectangle
    
            cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2); // Green color, 2px thickness

            // Prepare label text
            std::string label = classNames[classId] + ": " + cv::format("%.2f", confidence);

            // Get text size for background rectangle
            int baseLine;
            cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

            // // Draw background rectangle for text
            // cv::rectangle(frame, cv::Point(box.x, box.y - labelSize.height - baseLine),
            //               cv::Point(box.x + labelSize.width, box.y),
            //               cv::Scalar(0, 255, 0), cv::FILLED); // Green filled rectangle

            // Put text on the frame
            cv::putText(frame, label, cv::Point(box.x, box.y - baseLine),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1); // Black text
        }
    }
        void topic_callback(const sensor_msgs::msg::Image::SharedPtr msg)
    {

        static int counter{0};
        const float fps = 120.13;
        const int rate = 4;
        cv_bridge::CvImagePtr cv_ptr;
        try {
            cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::RGB8);
        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
            return; 
        }

        cv::Mat inverted_image;
        cv::Mat image = cv_ptr->image;
        
        ++counter;
        if(counter>fps/rate)
        {
            counter = 0;
            detectObjects(image);
        }

        addMarkers(image);


        cv_bridge::CvImage out_msg;
        out_msg.header = msg->header; 
        out_msg.encoding = sensor_msgs::image_encodings::RGB8;
        out_msg.image = inverted_image;
       

        publisher_->publish(*out_msg.toImageMsg());

        RCLCPP_INFO(this->get_logger(), "Published inverted image to /topic/B");
        
    }
};
    


RCLCPP_COMPONENTS_REGISTER_NODE(Yolo)

