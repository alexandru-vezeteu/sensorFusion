#include <iomanip>
#include <iostream>
#include <memory>
#include <sys/mman.h>
#include <thread>
#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>
#include <opencv2/opencv.hpp>
#include <libcamera/formats.h>
#include <boost/circular_buffer.hpp>

#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <filesystem>

using namespace libcamera;
using namespace std::chrono_literals;
static std::shared_ptr<Camera> camera1;
static boost::circular_buffer<cv::Mat> q1;
static unsigned int imageWidth1;
static unsigned int imageHeight1;
static unsigned int imageStride1;



static std::shared_ptr<Camera> camera2;
static boost::circular_buffer<cv::Mat> q2;
static unsigned int imageWidth2;
static unsigned int imageHeight2;
static unsigned int imageStride2;

std::atomic_bool cond {false};


std::shared_mutex mtx1, mtx2;

static void requestComplete1(Request *request)
{
    if(cond)
        return;
    if (request->status() == Request::RequestCancelled)
        return;

    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();
    for (auto bufferPair : buffers) {
        FrameBuffer *buffer = bufferPair.second;

        const FrameBuffer::Plane &plane = buffer->planes()[0];
        int fd = plane.fd.get();

        void *memory = mmap(NULL, plane.length, PROT_READ, MAP_SHARED, fd, 0);
        if (memory == MAP_FAILED) {
            std::cerr << "mmap failed" << std::endl;
            return;
        }

        unsigned int width = imageWidth1;
        unsigned int height = imageHeight1;
        unsigned int stride = imageStride1;

        cv::Mat rgbFrame(height, width, CV_8UC3, memory, stride);

        {
            std::unique_lock lck{mtx1};
            q1.push_back(rgbFrame.clone());
        }



        munmap(memory, plane.length);
    }

    request->reuse(Request::ReuseBuffers);
    camera1->queueRequest(request);
}


static void requestComplete2(Request *request)
{
    if(cond)
        return;
    if (request->status() == Request::RequestCancelled)
        return;

    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();
    for (auto bufferPair : buffers) {
        FrameBuffer *buffer = bufferPair.second;

        const FrameBuffer::Plane &plane = buffer->planes()[0];
        int fd = plane.fd.get();

        void *memory = mmap(NULL, plane.length, PROT_READ, MAP_SHARED, fd, 0);
        if (memory == MAP_FAILED) {
            std::cerr << "mmap failed" << std::endl;
            return;
        }

        unsigned int width = imageWidth2;
        unsigned int height = imageHeight2;
        unsigned int stride = imageStride2;

        cv::Mat rgbFrame(height, width, CV_8UC3, memory, stride);

       
        {
            std::unique_lock lck{mtx2};
            q2.push_back(rgbFrame.clone());
        }

        munmap(memory, plane.length);
    }

    request->reuse(Request::ReuseBuffers);
    camera2->queueRequest(request);
}



void display2Cameras()
{
    cv::namedWindow("Disparity", cv::WINDOW_NORMAL);
    cv::namedWindow("Rectified Stereo Pair", cv::WINDOW_NORMAL);
    cv::namedWindow("Depth Map", cv::WINDOW_NORMAL);

    cv::Mat m1, m2;

    // Load stereo calibration parameters
    cv::Mat K1, D1, K2, D2, R, T, R1, R2, P1, P2, Q;
    cv::FileStorage fs("stereocalibration_parameters.yaml", cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "Error: Could not open stereocalibration_parameters.yaml" << std::endl;
        return;
    }
    fs["K_left"] >> K1;
    fs["D_left"] >> D1;
    fs["K_right"] >> K2;
    fs["D_right"] >> D2;
    fs["R"] >> R;
    fs["T"] >> T;
    fs.release();

    // Wait for frames
    while (q1.empty() || q2.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    {
        std::unique_lock lck1{mtx1};
        m1 = q1.front();
    }
    {
        std::unique_lock lck2{mtx2};
        m2 = q2.front();
    }

    cv::Size image_size = m1.size();
    std::cout << "Using image size: " << image_size << std::endl;

    // Rectification
    cv::stereoRectify(K1, D1, K2, D2, image_size, R, T, R1, R2, P1, P2, Q,
                      cv::CALIB_ZERO_DISPARITY, 0.5, image_size);

    cv::Mat map1x, map1y, map2x, map2y;
    cv::initUndistortRectifyMap(K1, D1, R1, P1, image_size, CV_32FC1, map1x, map1y);
    cv::initUndistortRectifyMap(K2, D2, R2, P2, image_size, CV_32FC1, map2x, map2y);

    // Lightweight StereoBM
    int numDisparities = 16 * 10; // multiple of 16
    int blockSize = 9;           // odd number
    auto stereo = cv::StereoBM::create(numDisparities, blockSize);

    while (true)
    {
        {
            std::unique_lock lck1{mtx1};
            if (!q1.empty()) {
                m1 = q1.front();
                q1.pop_front();
            }
        }
        {
            std::unique_lock lck2{mtx2};
            if (!q2.empty()) {
                m2 = q2.front();
                q2.pop_front();
            }
        }

        if (!m1.empty() && !m2.empty())
        {
            if (m1.size() != image_size) cv::resize(m1, m1, image_size);
            if (m2.size() != image_size) cv::resize(m2, m2, image_size);

            // Rectify
            cv::Mat left_rect, right_rect;
            cv::remap(m1, left_rect, map1x, map1y, cv::INTER_LINEAR);
            cv::remap(m2, right_rect, map2x, map2y, cv::INTER_LINEAR);

            // Grayscale
            cv::Mat left_gray, right_gray;
            cv::cvtColor(left_rect, left_gray, cv::COLOR_BGR2GRAY);
            cv::cvtColor(right_rect, right_gray, cv::COLOR_BGR2GRAY);

            // Disparity map
            cv::Mat raw_disparity;
            stereo->compute(left_gray, right_gray, raw_disparity);

            // Convert to float
            cv::Mat disparity;
            raw_disparity.convertTo(disparity, CV_32F, 1.0 / 16.0);

            // Disparity visualization
            cv::Mat disp_vis;
            cv::normalize(disparity, disp_vis, 0, 255, cv::NORM_MINMAX, CV_8U);
            cv::applyColorMap(disp_vis, disp_vis, cv::COLORMAP_JET);
            cv::imshow("Disparity", disp_vis);

            // // Filter invalid values
            cv::Mat valid_mask = disparity > 0;

            // Reproject to 3D
            cv::Mat depth_map;
            cv::reprojectImageTo3D(disparity, depth_map, Q, true);
            std::vector<cv::Mat> xyz;
            cv::split(depth_map, xyz);
            cv::Mat depth = xyz[2];

            // Filter & visualize depth
            cv::Mat filtered_depth;
            depth.copyTo(filtered_depth, valid_mask);

            cv::Mat depth_vis;
            cv::normalize(filtered_depth, depth_vis, 0, 255, cv::NORM_MINMAX, CV_8U);
            cv::applyColorMap(depth_vis, depth_vis, cv::COLORMAP_JET);
            cv::imshow("Depth Map", depth_vis);

            // Draw epipolar lines
            for (int y = 0; y < image_size.height; y += 20) {
                cv::line(left_rect, cv::Point(0, y), cv::Point(image_size.width, y), cv::Scalar(0, 255, 0), 1);
                cv::line(right_rect, cv::Point(0, y), cv::Point(image_size.width, y), cv::Scalar(0, 255, 0), 1);
            }

            // Show stereo pair
            cv::Mat stereo_combined;
            cv::hconcat(left_rect, right_rect, stereo_combined);
            cv::imshow("Rectified Stereo Pair", stereo_combined);

            // Log disparity
            double minVal, maxVal;
            cv::minMaxLoc(disparity, &minVal, &maxVal);
            std::cout << "Disparity min: " << minVal << ", max: " << maxVal << std::endl;
        }

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q' || key == 'Q') {
            cv::destroyAllWindows();
            cond = true;
            cond.notify_all();
            return;
        }
    }
}



int main(int argc, char** argv)
{
    q1 = boost::circular_buffer<cv::Mat>(5);
    q2 = boost::circular_buffer<cv::Mat>(5);
    if (argc < 7) 
    {
        std::cerr << "Usage: " << argv[0] << " id width height id width height" << std::endl;
        return -1;
    }

    int cameraNumber1{}, width1{}, height1{};
    int cameraNumber2{}, width2{}, height2{};
    
    try
    {
        cameraNumber1 = std::stoi(argv[1]);
        width1 = std::stoi(argv[2]);
        height1 = std::stof(argv[3]);

        cameraNumber2 = std::stoi(argv[4]);
        width2 = std::stoi(argv[5]);
        height2 = std::stof(argv[6]);
    }
    catch(...)
    {
        std::cerr << "Usage: " << argv[0] << " id width height id width height" << std::endl;
        return -1;
    }



    std::filesystem::path pics_dir = "pics";
    std::filesystem::path left = pics_dir/"left";
    std::filesystem::path right = pics_dir/"right";
    std::filesystem::path p {};
    auto vec = {pics_dir, left, right};
    try 
    {
        
        for(auto& path : vec)
        {
            p = path;
            if (!std::filesystem::exists(path)) 
            {
                std::filesystem::create_directory(path);
                std::cout << "Created directory: " << path << std::endl;
            }
            else 
            {
                std::cout << "Directory already exists: " << path << std::endl;
            }
        }
    } 
    catch (const std::filesystem::filesystem_error& e) 
    {
        std::cerr << "Error creating directory " << p<<": " << e.what() << std::endl;
        return -1;
    }

    // Code to follow
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();
    
    
    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::cout << "No cameras were identified on the system."
                << std::endl;
        cm->stop();
        return EXIT_FAILURE;
    }

    std::string cameraId = cameras[cameraNumber1]->id();
    camera1 = cm->get(cameraId);

    cameraId = cameras[cameraNumber2]->id();
    camera2 = cm->get(cameraId);

    camera1->acquire();
    camera2->acquire();

    std::unique_ptr<CameraConfiguration> config1 = 
        camera1->generateConfiguration( { StreamRole::VideoRecording } );
    std::unique_ptr<CameraConfiguration> config2 = 
        camera2->generateConfiguration( { StreamRole::VideoRecording } );
    
    StreamConfiguration &streamConfig1 = config1->at(0);
    config1->at(0).pixelFormat = libcamera::PixelFormat(libcamera::formats::RGB888);
    config1->at(0).size.height = height1;
    config1->at(0).size.width = width1;
    config1->validate();
    camera1->configure(config1.get());

    StreamConfiguration &streamConfig2 = config2->at(0);
    config2->at(0).pixelFormat = libcamera::PixelFormat(libcamera::formats::RGB888);
    config2->at(0).size.height = height2;
    config2->at(0).size.width = width2;
    config2->validate();
    camera2->configure(config2.get());



    imageWidth1 = streamConfig1.size.width;
    imageHeight1 = streamConfig1.size.height;
    imageStride1 = streamConfig1.stride;

    imageWidth2 = streamConfig2.size.width;
    imageHeight2 = streamConfig2.size.height;
    imageStride2 = streamConfig2.stride;


    FrameBufferAllocator *allocator1 = new FrameBufferAllocator(camera1);
    FrameBufferAllocator *allocator2 = new FrameBufferAllocator(camera2);


    for (StreamConfiguration &cfg : *config1) {
        int ret = allocator1->allocate(cfg.stream());
        if (ret < 0) {
            std::cerr << "Can't allocate buffers" << std::endl;
            return -ENOMEM;
        }

        size_t allocated = allocator1->buffers(cfg.stream()).size();
        //std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
    }
    for (StreamConfiguration &cfg : *config2) {
        int ret = allocator2->allocate(cfg.stream());
        if (ret < 0) {
            std::cerr << "Can't allocate buffers" << std::endl;
            return -ENOMEM;
        }

        size_t allocated = allocator2->buffers(cfg.stream()).size();
        //std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
    }
    
    
    Stream *stream1 = streamConfig1.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers1 = allocator1->buffers(stream1);
    std::vector<std::unique_ptr<Request>> requests1;

    Stream *stream2 = streamConfig2.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers2 = allocator2->buffers(stream2);
    std::vector<std::unique_ptr<Request>> requests2;

    for (unsigned int i = 0; i < buffers1.size(); ++i) {
        std::unique_ptr<Request> request = camera1->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            return -ENOMEM;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers1[i];
        int ret = request->addBuffer(stream1, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request"
                << std::endl;
            return ret;
        }

        requests1.push_back(std::move(request));
    }


    for (unsigned int i = 0; i < buffers2.size(); ++i) {
        std::unique_ptr<Request> request = camera2->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            return -ENOMEM;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers2[i];
        int ret = request->addBuffer(stream2, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request"
                << std::endl;
            return ret;
        }

        requests2.push_back(std::move(request));
    }
    

    camera1->requestCompleted.connect(requestComplete1);
    camera2->requestCompleted.connect(requestComplete2);
    camera1->start();
    camera2->start();
    for (std::unique_ptr<Request> &request : requests1)
        camera1->queueRequest(request.get());

    for (std::unique_ptr<Request> &request : requests2)
        camera2->queueRequest(request.get());




    std::thread t(display2Cameras);
    
    cond.wait(true);
        
    
    t.join();
    camera1->stop();
    allocator1->free(stream1);
    delete allocator1;
    camera1->release();
    camera1.reset();

    camera2->stop();
    allocator2->free(stream2);
    delete allocator2;
    camera2->release();
    camera2.reset();
    
    cm->stop();
    return 0;
}