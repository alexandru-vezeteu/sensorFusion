
#include "../include/sensorFusion/BlurFilter.hpp"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <opencv2/core.hpp>
#include <cv_bridge/cv_bridge.h>
#include <rclcpp_components/register_node_macro.hpp>



BlurFilter::BlurFilter(const rclcpp::NodeOptions & options) : rclcpp::Node("blurFilter", options)
{
     publisher_ = this->create_publisher<sensor_msgs::msg::Image>("/sensorFusion/blur_filered", 10);

        subscriber_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera",
            10,
            std::bind(&BlurFilter::topic_callback, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "Filter node has been initialized!");
        RCLCPP_INFO(this->get_logger(), "Subscribing to /camera and publishing to /sensorFusion/blur_filered");
}

void BlurFilter::topic_callback(const sensor_msgs::msg::Image::SharedPtr msg) const
{

    
    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::RGB8);
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return; 
    }

    cv::Mat inverted_image;
    
    cv::bitwise_not(cv_ptr->image, inverted_image);

    cv_bridge::CvImage out_msg;
    out_msg.header = msg->header; 
    out_msg.encoding = sensor_msgs::image_encodings::RGB8;
    out_msg.image = inverted_image;

    publisher_->publish(*out_msg.toImageMsg());

    RCLCPP_INFO(this->get_logger(), "Published inverted image to /topic/B");
}
