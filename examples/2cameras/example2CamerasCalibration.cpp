#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <opencv2/opencv.hpp>


int main(int argc, char** argv)
{
    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " <left_image_folder> <left_parameters.yaml> <right_image_folder> <right_parameters.yaml> rows columns size(mm)" << std::endl;
        return -1;
    }

    std::string left_folder = argv[1];
    std::string right_folder = argv[3];
    std::filesystem::path left_parameters = argv[2];
    std::filesystem::path right_parameters = argv[4];

    if(!(std::filesystem::exists(left_folder)       &&
        std::filesystem::exists(right_folder)       &&
        std::filesystem::exists(left_parameters)    &&
        std::filesystem::exists(right_parameters)
    ))
    {
        std::cerr<<"Make sure all the paths are valid."<<std::endl;
        std::cerr << "Usage: " << argv[0] << " <left_image_folder> <left_parameters.yaml> <right_image_folder> <right_parameters.yaml> rows columns size(mm)" << std::endl;
        return -1;
    }

    int rows{}, columns{};
    float SQUARE_SIZE{};
    try
    {
        rows = std::stoi(argv[5]);
        columns=std::stoi(argv[6]);
        SQUARE_SIZE = std::stof(argv[7]);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Usage: " << argv[0] << " <left_image_folder> <left_parameters.yaml> <right_image_folder> <right_parameters.yaml> rows columns size(mm)" << std::endl;
    }

    cv::Mat K1, D1, K2, D2, R, T, E, F;

    cv::FileStorage f1(left_parameters, cv::FileStorage::READ);
    cv::FileStorage f2(right_parameters, cv::FileStorage::READ);
    f1["camera_matrix"]>>K1;
    f1["dist_coeffs"]>>D1;
    f2["camera_matrix"]>>K2;
    f2["dist_coeffs"]>>D2;
    f1.release();
    f2.release();


    const cv::Size CHECKERBOARD_DIMENSIONS(columns, rows);

    std::vector<cv::String> left_images, right_images;
    cv::glob(left_folder, left_images);
    cv::glob(right_folder, right_images);

    std::sort(left_images.begin(), left_images.end());
    std::sort(right_images.begin(), right_images.end());


    if (left_images.size() != right_images.size() || left_images.empty()) 
    {
        std::cerr << "Error: Number of left and right images must be equal and non-zero." << std::endl;
        return -1;
    }

    std::vector<cv::Point3f> object_points_template;
    for (int i = 0; i < CHECKERBOARD_DIMENSIONS.height; i++) {
        for (int j = 0; j < CHECKERBOARD_DIMENSIONS.width; j++) {
            object_points_template.push_back(cv::Point3f(j * SQUARE_SIZE, i * SQUARE_SIZE, 0));
        }
    }

    std::vector<std::vector<cv::Point3f>> object_points;
    std::vector<std::vector<cv::Point2f>> left_image_points, right_image_points;
    cv::Size image_size;

    std::cout << "Finding checkerboard corners..." << std::endl;
    for (size_t i = 0; i < left_images.size(); i++) {
        cv::Mat left_img = cv::imread(left_images[i]);
        cv::Mat right_img = cv::imread(right_images[i]);
        
        

        if (left_img.empty() || right_img.empty()) {
            continue;
        }

        image_size = left_img.size();
        std::vector<cv::Point2f> left_corners, right_corners;
        bool found_left = cv::findChessboardCorners(left_img, CHECKERBOARD_DIMENSIONS, left_corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
        bool found_right = cv::findChessboardCorners(right_img, CHECKERBOARD_DIMENSIONS, right_corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

        if (found_left && found_right) {
            cv::Mat left_gray, right_gray;
            cv::cvtColor(left_img, left_gray, cv::COLOR_BGR2GRAY);
            cv::cvtColor(right_img, right_gray, cv::COLOR_BGR2GRAY);

            cv::cornerSubPix(left_gray, left_corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1));
            cv::cornerSubPix(right_gray, right_corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1));

            left_image_points.push_back(left_corners);
            right_image_points.push_back(right_corners);
            object_points.push_back(object_points_template);

            std::cout << "Processed image pair " << i + 1 << std::endl;
        } else {
            std::cout << "Corners not found in image pair " << i + 1 << std::endl;
        }
    }

    if (object_points.empty()) {
        std::cerr << "No valid image pairs found for calibration." << std::endl;
        return -1;
    }

    std::cout << "\nStarting stereocalibration..." << std::endl;

    double rms = cv::stereoCalibrate(object_points, left_image_points, right_image_points,
                                      K1, D1, K2, D2, image_size, R, T, E, F,
                                      cv::CALIB_FIX_INTRINSIC,
                                      cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 100, 1e-5));
    

    std::cout << "RMS error: " << rms << std::endl;



    cv::FileStorage fs("stereocalibration_parameters.yaml", cv::FileStorage::WRITE);
    if (fs.isOpened())
    {
        fs << "K_left" << K1;
        fs << "D_left" << D1;
        fs << "K_right" << K2;
        fs << "D_right" << D2;
        fs << "R" << R;
        fs << "T" << T;
        fs.release();
        std::cout << "\nParameters saved to stereocalibration_parameters.yml" << std::endl;
    }
    else
    {
        std::cerr << "Error: Could not open the file to save parameters." << std::endl;
    }

    return 0;
}