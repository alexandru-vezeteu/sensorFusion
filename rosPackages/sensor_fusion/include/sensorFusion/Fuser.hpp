#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <message_filters/subscriber.hpp>
#include <message_filters/sync_policies/approximate_time.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

using LaserScan=sensor_msgs::msg::LaserScan;

using FuserPolicy = message_filters::sync_policies::ApproximateTime<LaserScan, LaserScan>;

namespace sensorFusion
{
    class Fuser : public rclcpp::Node
    {
    public:

        Fuser(const rclcpp::NodeOptions & options);

    private:
        void sync_callback(
            const LaserScan::ConstSharedPtr& left_msg, 
            const LaserScan::ConstSharedPtr& right_msg) const;


        rclcpp::Publisher<LaserScan>::SharedPtr publisher_;

        std::unique_ptr<message_filters::Subscriber<LaserScan>> subscriberLidar_;
        std::unique_ptr<message_filters::Subscriber<LaserScan>> subscriberTriangulation_;
        
        std::shared_ptr<message_filters::Synchronizer<FuserPolicy>> sync_;
    };
}






RCLCPP_COMPONENTS_REGISTER_NODE(sensorFusion::Fuser)
