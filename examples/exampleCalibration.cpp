#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

std::vector<std::string> getImageFiles(const std::string& directoryPath) 
{
    std::vector<std::string> filePaths;
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) 
    {
        if (entry.is_regular_file() ) 
        {
            
            auto extension = entry.path().extension();
            std::cout<<extension<<std::endl;
            
            
            if (!extension.compare(".jpg")  || 
                !extension.compare(".png")  || 
                !extension.compare(".jpeg") || 
                !extension.compare(".bmp"))
            {
                filePaths.push_back(entry.path());
            }
        }
    }
    
    return filePaths;
}

int main(int argc, char** argv) 
{
    if (argc < 5) 
    {
        std::cerr << "Usage: " << argv[0] << "path/to/chessboard/images/folder rows columns size(in mm)" << std::endl;
        return -1;
    }

    std::string imageFolder = argv[1];
    int rows{}, columns{};
    float squareSize{};
    
    try
    {
        rows = std::stoi(argv[2]);
        columns = std::stoi(argv[3]);
        squareSize = std::stof(argv[4]);
    }
    catch(...)
    {
        std::cerr << "Usage: " << argv[0] << "path/to/chessboard/images/folder rows columns size(in mm)" << std::endl;
        std::cerr<<"rows, columns\t-\tintegers"<<std::endl<<"size\t\t-\tfloating point"<<std::endl;
        return -1;
    }
    
    cv::Size boardSize(columns, rows);
    
    std::vector<std::vector<cv::Point3f>> objectPoints;
    std::vector<std::vector<cv::Point2f>> imagePoints;

    std::vector<cv::Point3f> obj;
    for (int i = 0; i < boardSize.height; ++i) 
    {
        for (int j = 0; j < boardSize.width; ++j) 
        {
            obj.push_back(cv::Point3f(j * squareSize, i * squareSize, 0.0f));
        }
    }

    std::vector<std::string> imageFiles = getImageFiles(imageFolder);
    if (imageFiles.empty()) 
    {
        std::cerr << "No image files found in the specified folder: " << imageFolder << std::endl;
        return -1;
    }

    std::vector<cv::Point2f> corners;
    int successCount = 0;

    cv::Mat image{};
    cv::namedWindow("Corners Found", cv::WINDOW_NORMAL);
    for (const auto& filePath : imageFiles)
    {
        image = cv::imread(filePath, cv::IMREAD_GRAYSCALE);
        if (image.empty())
        {
            std::cerr << "Warning: Could not open or find the image " << filePath << std::endl;
            continue;
        }


        bool found = cv::findChessboardCorners(image, boardSize, corners,
                                               0
                                               | cv::CALIB_CB_ADAPTIVE_THRESH
                                               //| cv::CALIB_CB_NORMALIZE_IMAGE
                                               //| cv::CALIB_CB_FAST_CHECK
                                            );

        if (found)
        {
            cv::cornerSubPix(image, corners, cv::Size(11, 11), cv::Size(-1, -1),
                             cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001));

            imagePoints.push_back(corners);
            objectPoints.push_back(obj);
            successCount++;

            cv::drawChessboardCorners(image, boardSize, cv::Mat(corners), found);
            cv::imshow("Corners Found", image);
            cv::waitKey(50);
        } else 
        {
            std::cout << "Could not find chessboard corners in: " << filePath << std::endl;
        }
    }

    cv::destroyAllWindows();

    if (successCount == 0) 
    {
        std::cerr << "Error: No chessboard corners were successfully detected in any image. Calibration aborted." << std::endl;
        return -1;
    }

    std::cout << "Successfully detected corners in " << successCount << " images out of " << imageFiles.size() << "." << std::endl;
    std::cout << "Calibrating camera..." << std::endl;

    cv::Mat cameraMatrix, distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs;

    double reprojectionError = cv::calibrateCamera(objectPoints, imagePoints, image.size(), cameraMatrix, distCoeffs, rvecs, tvecs, cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5);

    std::cout << "\nCamera Calibration Results:" << std::endl;
    std::cout << "Camera Matrix (Intrinsic Parameters):\n" << cameraMatrix << std::endl;
    std::cout << "\nDistortion Coefficients:\n" << distCoeffs << std::endl;
    std::cout << "\nTotal Reprojection Error: " << reprojectionError << std::endl;

    cv::FileStorage fs("camera_calibration_results.yml", cv::FileStorage::WRITE);
    if (fs.isOpened()) 
    {
        fs << "camera_matrix" << cameraMatrix;
        fs << "dist_coeffs" << distCoeffs;
        fs << "reprojection_error" << reprojectionError;
        fs.release();
        std::cout << "\nCalibration results saved to camera_calibration_results.yml" << std::endl;
    } else 
    {
        std::cerr << "Error: Could not open file for saving calibration results." << std::endl;
    }

    if (!imageFiles.empty()) 
    {
        cv::Mat testImage = cv::imread(imageFiles[0]);
        if (!testImage.empty()) 
        {
            cv::Mat undistortedImage;
            cv::undistort(testImage, undistortedImage, cameraMatrix, distCoeffs);
            cv::namedWindow("Original Image", cv::WINDOW_NORMAL);
            cv::namedWindow("Undistorted Image", cv::WINDOW_NORMAL);
            cv::imshow("Original Image", testImage);
            cv::imshow("Undistorted Image", undistortedImage);
            cv::waitKey(0);
        } else 
        {
            std::cerr << "Warning: Could not load test image for undistortion." << std::endl;
        }
    }
    

    return 0;
}
