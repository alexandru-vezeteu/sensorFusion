
#include "../include/sensorFusion/NoiseFilter.hpp"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <opencv2/core.hpp>
#include <cv_bridge/cv_bridge.h>
#include <rclcpp_components/register_node_macro.hpp>
#include <rclcpp/qos.hpp>

using namespace sensorFusion;

NoiseFilter::NoiseFilter(const rclcpp::NodeOptions & options) : rclcpp::Node("noiseFilter", options)
{
    rclcpp::QoS qos_profile(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default));
    qos_profile.keep_last(1);
    qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);

    publisher_ = this->create_publisher<sensor_msgs::msg::Image>("/sensor_fusion/noise_filter_out", qos_profile);

    subscriber_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/sensor_fusion/noise_filter_in",
            qos_profile,
            std::bind(&NoiseFilter::topic_callback, this, std::placeholders::_1)
        );

    RCLCPP_INFO(this->get_logger(), "Noise Filter node has been initialized!");
    RCLCPP_INFO(this->get_logger(), "Subscribing to /sensor_fusion/noise_filter_in and publishing to /sensor_fusion/noise_filter_out");
}

void NoiseFilter::topic_callback(const sensor_msgs::msg::Image::SharedPtr msg) const
{

    
    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::RGB8);
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return; 
    }

    

    std::vector<cv::Mat> channels;
    cv::split(cv_ptr->image, channels);

    for(int i=0;i<3;++i)
        cv::equalizeHist(channels[i], channels[i]);

    cv::merge(channels, cv_ptr->image);
    cv::Mat equalized_image = cv_ptr->image;

    cv::Mat denoised_image;
    cv::medianBlur(equalized_image, denoised_image, 3);

    cv_bridge::CvImage out_msg;
    out_msg.header = msg->header; 
    out_msg.encoding = sensor_msgs::image_encodings::RGB8;
    out_msg.image = denoised_image;

    publisher_->publish(*out_msg.toImageMsg());

}
