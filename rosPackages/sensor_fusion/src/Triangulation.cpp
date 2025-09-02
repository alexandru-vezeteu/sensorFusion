
#include "../include/sensorFusion/Triangulation.hpp"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <rclcpp_components/register_node_macro.hpp>

#include <sensor_fusion_messages/msg/detection.hpp>

#include <ranges>
using namespace sensorFusion;

Triangulation::Triangulation(const rclcpp::NodeOptions & options) : rclcpp::Node("triangulation", options)
{
    declare_parameter<std::string>("path_to_calib", "/ros_ws/params.yaml");
    declare_parameter<int>("image_width", 1280);
    declare_parameter<int>("image_height", 960);
    declare_parameter<double>("matching_treshold", 10.0);
    declare_parameter<double>("min_angle", 0.0);


    cv::FileStorage fs( get_parameter("path_to_calib").as_string(),
                        cv::FileStorage::READ);
    
    fs["K_left"]    >> K_left;
    fs["K_right"]   >> K_right;
    fs["D_left"]    >> D_left;
    fs["D_right"]   >> D_right;
    fs["R"]         >> R;
    fs["T"]         >> T;

    fs.release();

    image_size = {  static_cast<int>(get_parameter("image_width").as_int()),
                    static_cast<int>(get_parameter("image_height").as_int())};

    cv::stereoRectify(  K_left, D_left, K_right, D_right, image_size, R, T, R1, 
                        R2, P1, P2, Q,
                        cv::CALIB_ZERO_DISPARITY, 0.5, image_size
                        );
    
    cv::initUndistortRectifyMap(K_left, D_left, R1, P1, image_size, CV_32FC1, map1x, map1y);
    cv::initUndistortRectifyMap(K_right, D_right, R2, P2, image_size, CV_32FC1, map2x, map2y);


    publisher_ = this->create_publisher<LaserScan>("/sensorFusion/triangulation_out", 10);

    subscriberLeft_ = std::make_unique<message_filters::Subscriber<Detection>>(this, "/sensor_fusion/triangulation_in_left");
    subscriberRight_ = std::make_unique<message_filters::Subscriber<Detection>>(this, "/sensor_fusion/triangulation_in_right");
    
    

    
    sync_ = std::make_shared<message_filters::Synchronizer<DetectionPolicy>>(
        DetectionPolicy(4), *subscriberLeft_, *subscriberRight_
    );
    sync_->setMaxIntervalDuration(rclcpp::Duration::from_seconds(0.2));


    sync_->registerCallback(
            std::bind(&Triangulation::sync_callback, this, std::placeholders::_1, std::placeholders::_2)
        );

    RCLCPP_INFO(this->get_logger(), "Triangulation node has been initialized!");
    RCLCPP_INFO(this->get_logger(), "Subscribing to /sensor_fusion/triangulation_in_left and /sensor_fusion/triangulation_in_right and publishing to /sensor_fusion/triangulation_out");
}

void Triangulation::sync_callback(
            const Detection::ConstSharedPtr& left_msg, 
            const Detection::ConstSharedPtr& right_msg)
{
    
    std::vector<std::pair<
                        sensor_fusion_messages::msg::BoundingBox, 
                        sensor_fusion_messages::msg::BoundingBox>
                        > matched_boxes;
    const double y_threshold { get_parameter("matching_treshold").as_double() };

    std::vector<bool> right_used(right_msg->boxes.size(), false);

    for (const auto& left_box : left_msg->boxes)
    {
        double left_cx = (left_box.left_up.x + left_box.right_down.x) / 2.0;
        double left_cy = (left_box.left_up.y + left_box.right_down.y) / 2.0;

        double best_y_diff = std::numeric_limits<double>::max();
        size_t best_index = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < right_msg->boxes.size(); ++i)
        {
            if (right_used[i])
                continue;

            const auto& right_box = right_msg->boxes[i];
            double right_cx = (right_box.left_up.x + right_box.right_down.x) / 2.0;
            double right_cy = (right_box.left_up.y + right_box.right_down.y) / 2.0;

            double y_diff = std::abs(left_cy - right_cy);

            if (y_diff < y_threshold && y_diff < best_y_diff)
            {
                best_y_diff = y_diff;
                best_index = i;
            }
        }

        if (best_index != std::numeric_limits<size_t>::max())
        {
            matched_boxes.emplace_back(std::make_pair(left_box, right_msg->boxes[best_index]));
            right_used[best_index] = true;
        }
    }

    std::vector<cv::Point3f> triangulated_points;

    for (const auto& [left_box, right_box] : matched_boxes)
    {
        int left_y = static_cast<int>((left_box.left_up.y + left_box.right_down.y) / 2.0);
        int left_start_x = static_cast<int>(left_box.left_up.x);
        int left_end_x   = static_cast<int>(left_box.right_down.x);

        int right_y = static_cast<int>((right_box.left_up.y + right_box.right_down.y) / 2.0);
        int right_start_x = static_cast<int>(right_box.left_up.x);
        int right_end_x   = static_cast<int>(right_box.right_down.x);

        int line_length = std::min(left_end_x - left_start_x, right_end_x - right_start_x);
        if (line_length <= 0) continue;

        // Prepare vectors of points along the center line
        std::vector<cv::Point2f> pts_left, pts_right;
        pts_left.reserve(line_length);
        pts_right.reserve(line_length);

        for (int i = 0; i < line_length; ++i)
        {
            pts_left.emplace_back(left_start_x + i, left_y);
            pts_right.emplace_back(right_start_x + i, right_y);
        }

        // Triangulate all points at once
        cv::Mat points_4d;
        cv::triangulatePoints(P1, P2, pts_left, pts_right, points_4d);

        // Convert homogeneous coordinates to 3D points
        cv::Mat points_3d;
        cv::convertPointsFromHomogeneous(points_4d.t(), points_3d);

        // Add all points to the output vector
        for (int i = 0; i < points_3d.rows; ++i)
        {
            cv::Point3f pt_3d = points_3d.at<cv::Point3f>(i);
            triangulated_points.push_back(pt_3d);
        }
    }

    
    auto& p1 = *triangulated_points.begin();
    auto& p2 = *(triangulated_points.begin()+triangulated_points.size()-1);

    auto it = std::remove_if(triangulated_points.begin(), triangulated_points.end(),
            [](cv::Point3f& pt){return pt.z<0;});
    
    triangulated_points.erase(it, triangulated_points.end());

    RCLCPP_INFO(this->get_logger(), "nr de puncte: %zu", triangulated_points.size());

    LaserScan scan_msg;
    scan_msg.header.stamp = this->get_clock()->now();
    scan_msg.header.frame_id = "laser";

    scan_msg.angle_min = -M_PI;      // -180°
    scan_msg.angle_max = M_PI;       // +180°
    scan_msg.angle_increment = M_PI / 360.0;


    scan_msg.range_min = 0.1;
    scan_msg.range_max = 30.0;

    size_t num_bins = std::ceil((scan_msg.angle_max - scan_msg.angle_min) / scan_msg.angle_increment);

    scan_msg.ranges.assign(num_bins, std::numeric_limits<float>::infinity());

    for (const auto& pt : triangulated_points)
    {
        float x = pt.x / 1000.0f;  // mm to meters
        float z = pt.z / 1000.0f;

        float range = std::sqrt(x * x + z * z);
        float angle = std::atan2(-z, x);  // Angle in radians, 0 = forward

        // Filter out invalid angles (shouldn't happen, but safe)
        if (angle < scan_msg.angle_min || angle >= scan_msg.angle_max)
            continue;

        size_t index = static_cast<size_t>((angle - scan_msg.angle_min) / scan_msg.angle_increment);
        if (index >= num_bins)
            continue;

        if (range < scan_msg.ranges[index])
            scan_msg.ranges[index] = range;
    }

    publisher_->publish(scan_msg);
}
        


