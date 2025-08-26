
#include "../include/sensorFusion/Fuser.hpp"

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>


#include <sensor_msgs/point_cloud2_iterator.hpp>

using namespace sensorFusion;

Fuser::Fuser(const rclcpp::NodeOptions & options) : rclcpp::Node("fuser", options)
{

    publisher_ = this->create_publisher<PointCloud>("/sensor_fusion/fuser_out", 10);

    subscriberLidar_ = this->create_subscription<LaserScan>("/sensor_fusion/fuser_in_lidar", 
                                                            10, 
                                                            std::bind(&Fuser::lidar_callback, this, std::placeholders::_1));

    subscriberTriangulation_ = this->create_subscription<LaserScan>("/sensor_fusion/fuser_in_triangulation", 
                                                            10, 
                                                            std::bind(&Fuser::triangulation_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Noise Filter node has been initialized!");
    RCLCPP_INFO(this->get_logger(), "Subscribing to sensor_fusion/laserScan_lidar, /sensor_fusion/laserScan_triangulation and publishing to /sensor_fusion/fuser_out");
}


void Fuser::lidar_callback(const LaserScan msg) const
{
    auto cloud = std::make_shared<PointCloud>();
    cloud->header = msg.header;
    cloud->height = 1;
    cloud->width = msg.ranges.size();
    cloud->is_dense = false;


    
    sensor_msgs::PointCloud2Modifier modifier(*cloud);
    modifier.setPointCloud2FieldsByString(2, "xyz", "rgb");

    sensor_msgs::PointCloud2Iterator<float> iter_x(*cloud, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(*cloud, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(*cloud, "z");
    sensor_msgs::PointCloud2Iterator<uint8_t> iter_r(*cloud, "r");
    sensor_msgs::PointCloud2Iterator<uint8_t> iter_g(*cloud, "g");
    sensor_msgs::PointCloud2Iterator<uint8_t> iter_b(*cloud, "b");

    float angle = msg.angle_min;

    for (size_t i = 0; i < msg.ranges.size();   ++i,
                                                ++iter_x, ++iter_y, ++iter_z,
                                                ++iter_r, ++iter_g, ++iter_b)
    {
        float range = msg.ranges[i];

        if (range < msg.range_min || range > msg.range_max || std::isnan(range))
        {
            *iter_x = *iter_y = *iter_z = std::numeric_limits<float>::quiet_NaN();
            *iter_r = *iter_g = *iter_b = 0;
        }
        else
        {
            float x = range * std::cos(angle);
            float y = range * std::sin(angle);
            *iter_x = x;
            *iter_y = y;
            *iter_z = 0.0f;
            *iter_r = 255; *iter_g = 0;   *iter_b = 0;   // Red
        }

        angle += msg.angle_increment;
    }

    publisher_->publish(*cloud);

}

void Fuser::triangulation_callback(const LaserScan msg) const
{
    auto cloud = std::make_shared<PointCloud>();
    cloud->header = msg.header;
    cloud->height = 1;
    cloud->width = msg.ranges.size();
    cloud->is_dense = false;


    
    sensor_msgs::PointCloud2Modifier modifier(*cloud);
    modifier.setPointCloud2FieldsByString(2, "xyz", "rgb");

    sensor_msgs::PointCloud2Iterator<float> iter_x(*cloud, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(*cloud, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(*cloud, "z");
    sensor_msgs::PointCloud2Iterator<uint8_t> iter_r(*cloud, "r");
    sensor_msgs::PointCloud2Iterator<uint8_t> iter_g(*cloud, "g");
    sensor_msgs::PointCloud2Iterator<uint8_t> iter_b(*cloud, "b");

    
}


