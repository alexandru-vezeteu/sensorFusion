#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <rclcpp_components/register_node_macro.hpp>

namespace sensorFusion
{
    class BlurFilter : public rclcpp::Node
    {
    public:

        BlurFilter(const rclcpp::NodeOptions & options);

    private:
        void topic_callback(const sensor_msgs::msg::Image::SharedPtr msg) const;
        


        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscriber_;
    };
}






RCLCPP_COMPONENTS_REGISTER_NODE(sensorFusion::BlurFilter)
