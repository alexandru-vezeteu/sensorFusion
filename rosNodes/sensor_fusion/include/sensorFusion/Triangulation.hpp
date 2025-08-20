#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <message_filters/subscriber.hpp>
#include <message_filters/sync_policies/approximate_time.hpp>

#include <sensor_fusion_messages/msg/detection.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

using Detection=sensor_fusion_messages::msg::Detection;
using LaserScan=sensor_msgs::msg::LaserScan;

using DetectionPolicy = message_filters::sync_policies::ApproximateTime<Detection, Detection>;

namespace sensorFusion
{
    class Triangulation : public rclcpp::Node
    {
    public:

        Triangulation(const rclcpp::NodeOptions & options);

    private:
        void sync_callback(
            const Detection::ConstSharedPtr& left_msg, 
            const Detection::ConstSharedPtr& right_msg) const;


        rclcpp::Publisher<LaserScan>::SharedPtr publisher_;

        std::unique_ptr<message_filters::Subscriber<Detection>> subscriberLeft_;
        std::unique_ptr<message_filters::Subscriber<Detection>> subscriberRight_;
        
        std::shared_ptr<message_filters::Synchronizer<DetectionPolicy>> sync_;
    };
}






RCLCPP_COMPONENTS_REGISTER_NODE(sensorFusion::Triangulation)
