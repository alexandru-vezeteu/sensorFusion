
#include "../include/sensorFusion/Triangulation.hpp"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <opencv2/core.hpp>
#include <cv_bridge/cv_bridge.h>
#include <rclcpp_components/register_node_macro.hpp>

#include <sensor_fusion_messages/msg/detection.hpp>

using namespace sensorFusion;

Triangulation::Triangulation(const rclcpp::NodeOptions & options) : rclcpp::Node("triangulation", options)
{

    publisher_ = this->create_publisher<LaserScan>("/sensorFusion/triangulation_out", 10);

    subscriberLeft_ = std::make_unique<message_filters::Subscriber<Detection>>(this, "/sensor_fusion/yolo_left");
    subscriberRight_ = std::make_unique<message_filters::Subscriber<Detection>>(this, "/sensor_fusion/yolo_right");

    sync_ = std::make_shared<message_filters::Synchronizer<DetectionPolicy>>(
        DetectionPolicy(10), *subscriberLeft_, *subscriberRight_
    );


    sync_->registerCallback(
            std::bind(&Triangulation::sync_callback, this, std::placeholders::_1, std::placeholders::_2)
        );

    RCLCPP_INFO(this->get_logger(), "Noise Filter node has been initialized!");
    RCLCPP_INFO(this->get_logger(), "Subscribing to /sensorFusion/noise_filter_in and publishing to /sensorFusion/noise_filter_out");
}

void Triangulation::sync_callback(
            const Detection::ConstSharedPtr& left_msg, 
            const Detection::ConstSharedPtr& right_msg) const
{
    //do stuff
}
        


