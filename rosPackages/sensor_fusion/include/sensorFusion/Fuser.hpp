#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <message_filters/subscriber.hpp>
#include <message_filters/sync_policies/approximate_time.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

using LaserScan=sensor_msgs::msg::LaserScan;
using PointCloud = sensor_msgs::msg::PointCloud2;


namespace sensorFusion
{
    class Fuser : public rclcpp::Node
    {
    public:

        Fuser(const rclcpp::NodeOptions & options);

    private:
        void lidar_callback(const LaserScan msg) const;
        void triangulation_callback(const LaserScan msg) const;


        rclcpp::Publisher<PointCloud>::SharedPtr publisherTriangulation_;
        rclcpp::Publisher<PointCloud>::SharedPtr publisherLidar_;
        
        rclcpp::Subscription<LaserScan>::SharedPtr subscriberLidar_;
        rclcpp::Subscription<LaserScan>::SharedPtr subscriberTriangulation_;

        
    };
}






RCLCPP_COMPONENTS_REGISTER_NODE(sensorFusion::Fuser)
