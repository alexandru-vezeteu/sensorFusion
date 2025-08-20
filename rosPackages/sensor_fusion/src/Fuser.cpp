
#include "../include/sensorFusion/Fuser.hpp"

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>


using namespace sensorFusion;

Fuser::Fuser(const rclcpp::NodeOptions & options) : rclcpp::Node("fuser", options)
{

    publisher_ = this->create_publisher<LaserScan>("/sensorFusion/fuser_out", 10);

    subscriberLidar_ = std::make_unique<message_filters::Subscriber<LaserScan>>(this, "/sensor_fusion/laserScan_lidar");
    subscriberTriangulation_ = std::make_unique<message_filters::Subscriber<LaserScan>>(this, "/sensor_fusion/laserScan_triangulation");

    sync_ = std::make_shared<message_filters::Synchronizer<FuserPolicy>>(
        FuserPolicy(10), *subscriberLidar_, *subscriberTriangulation_
    );


    sync_->registerCallback(
            std::bind(&Fuser::sync_callback, this, std::placeholders::_1, std::placeholders::_2)
        );

    RCLCPP_INFO(this->get_logger(), "Noise Filter node has been initialized!");
    RCLCPP_INFO(this->get_logger(), "Subscribing to /sensorFusion/noise_filter_in and publishing to /sensorFusion/noise_filter_out");
}

void Fuser::sync_callback(
            const LaserScan::ConstSharedPtr& left_msg, 
            const LaserScan::ConstSharedPtr& right_msg) const
{
    //do stuff
}
        


