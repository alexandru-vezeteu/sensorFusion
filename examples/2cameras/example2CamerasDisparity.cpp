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



int numDisparities = 8;
int blockSize = 5;
int preFilterType = 1;
int preFilterSize = 1;
int preFilterCap = 31;
int minDisparity = 0;
int textureThreshold = 10;
int uniquenessRatio = 15;
int speckleRange = 0;
int speckleWindowSize = 0;
int disp12MaxDiff = -1;
int dispType = CV_16S;
 //https://learnopencv.com/depth-perception-using-stereo-camera-python-c/
// Creating an object of StereoSGBM algorithm
cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create();

static void on_trackbar1( int, void* )
{
  stereo->setNumDisparities(numDisparities*16);
  numDisparities = numDisparities*16;
}
 
static void on_trackbar2( int, void* )
{
  stereo->setBlockSize(blockSize*2+5);
  blockSize = blockSize*2+5;
}
 
static void on_trackbar3( int, void* )
{
  stereo->setPreFilterType(preFilterType);
}
 
static void on_trackbar4( int, void* )
{
  stereo->setPreFilterSize(preFilterSize*2+5);
  preFilterSize = preFilterSize*2+5;
}
 
static void on_trackbar5( int, void* )
{
  stereo->setPreFilterCap(preFilterCap);
}
 
static void on_trackbar6( int, void* )
{
  stereo->setTextureThreshold(textureThreshold);
}
 
static void on_trackbar7( int, void* )
{
  stereo->setUniquenessRatio(uniquenessRatio);
}
 
static void on_trackbar8( int, void* )
{
  stereo->setSpeckleRange(speckleRange);
}
 
static void on_trackbar9( int, void* )
{
  stereo->setSpeckleWindowSize(speckleWindowSize*2);
  speckleWindowSize = speckleWindowSize*2;
}
 
static void on_trackbar10( int, void* )
{
  stereo->setDisp12MaxDiff(disp12MaxDiff);
}
 
static void on_trackbar11( int, void* )
{
  stereo->setMinDisparity(minDisparity);
}
 
cv::Mat imgL;
cv::Mat imgR;
cv::Mat imgL_gray;
cv::Mat imgR_gray;



void display2Cameras()
{
    cv::namedWindow("disparity", cv::WINDOW_NORMAL);
    cv::namedWindow("Rectified Stereo Pair", cv::WINDOW_NORMAL);
    cv::namedWindow("Depth Map", cv::WINDOW_NORMAL);
    cv::createTrackbar("numDisparities", "disparity", &numDisparities, 18, on_trackbar1);
    cv::createTrackbar("blockSize", "disparity", &blockSize, 50, on_trackbar2);
    cv::createTrackbar("preFilterType", "disparity", &preFilterType, 1, on_trackbar3);
    cv::createTrackbar("preFilterSize", "disparity", &preFilterSize, 25, on_trackbar4);
    cv::createTrackbar("preFilterCap", "disparity", &preFilterCap, 62, on_trackbar5);
    cv::createTrackbar("textureThreshold", "disparity", &textureThreshold, 100, on_trackbar6);
    cv::createTrackbar("uniquenessRatio", "disparity", &uniquenessRatio, 100, on_trackbar7);
    cv::createTrackbar("speckleRange", "disparity", &speckleRange, 100, on_trackbar8);
    cv::createTrackbar("speckleWindowSize", "disparity", &speckleWindowSize, 25, on_trackbar9);
    cv::createTrackbar("disp12MaxDiff", "disparity", &disp12MaxDiff, 25, on_trackbar10);
    cv::createTrackbar("minDisparity", "disparity", &minDisparity, 25, on_trackbar11);
 

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
        m1 = q1.back();
        q1.pop_back();
    }
    {
        std::unique_lock lck2{mtx2};
        m2 = q2.back();
        q2.pop_back();
    }

    cv::Size image_size = m2.size();
    std::cout << "Using image size: " << image_size << std::endl;

    // Rectification
    cv::stereoRectify(K1, D1, K2, D2, image_size, R, T, R1, R2, P1, P2, Q,
                      cv::CALIB_ZERO_DISPARITY, 0.5, image_size);

    cv::Mat map1x, map1y, map2x, map2y;
    cv::initUndistortRectifyMap(K1, D1, R1, P1, image_size, CV_32FC1, map1x, map1y);
    cv::initUndistortRectifyMap(K2, D2, R2, P2, image_size, CV_32FC1, map2x, map2y);

    

    while (true)
    {
        {
            std::unique_lock lck1{mtx1};
            if (!q1.empty()) {
                m1 = q1.back();
                q1.pop_back();
            }
        }
        {
            std::unique_lock lck2{mtx2};
            if (!q2.empty()) {
                m2 = q2.back();
                q2.pop_back();
            }
        }

        if (!m1.empty() && !m2.empty())
        {
            if (m1.size() != image_size) cv::resize(m1, m1, image_size);
            if (m2.size() != image_size) cv::resize(m2, m2, image_size);


                        // Grayscale
            cv::Mat left_gray, right_gray;
            cv::cvtColor(m1, left_gray, cv::COLOR_BGR2GRAY);
            cv::cvtColor(m2, right_gray, cv::COLOR_BGR2GRAY);

            // Rectify
            cv::Mat left_rect, right_rect;
            cv::remap(left_gray, left_rect, map1x, map1y, cv::INTER_LANCZOS4, cv::BORDER_CONSTANT);
            cv::remap(right_gray, right_rect, map2x, map2y, cv::INTER_LANCZOS4, cv::BORDER_CONSTANT);



            // Disparity map
            cv::Mat raw_disparity;
            stereo->compute(left_rect, right_rect, raw_disparity);

            // Convert to float
            cv::Mat disparity;
            raw_disparity.convertTo(disparity, CV_32F, 1.0);
            disparity = (disparity/16.0f - (float)minDisparity)/((float)numDisparities);
            cv::Mat aux{};
            
            cv::imshow("disparity", disparity);

            
            cv::Mat points_3D;
            cv::reprojectImageTo3D(disparity, points_3D, Q, true);
            cv::Mat depth_map = cv::Mat(points_3D.size(), CV_32F);
            for (int y = 0; y < points_3D.rows; y++) {
                for (int x = 0; x < points_3D.cols; x++) {
                    cv::Vec3f point = points_3D.at<cv::Vec3f>(y, x);
                    depth_map.at<float>(y, x) = point[2]; // Z = depth
                }
            }
            cv::Mat depth_display;
            cv::normalize(depth_map, depth_display, 0, 255, cv::NORM_MINMAX);
            depth_display.convertTo(depth_display, CV_8U);
            cv::imshow("Depth Map", depth_display);

            // Draw epipolar lines
            for (int y = 0; y < image_size.height; y += 20) {
                cv::line(left_rect, cv::Point(0, y), cv::Point(image_size.width, y), cv::Scalar(0, 255, 0), 1);
                cv::line(right_rect, cv::Point(0, y), cv::Point(image_size.width, y), cv::Scalar(0, 255, 0), 1);
            }

            // Show stereo pair
            cv::Mat stereo_combined;
            cv::hconcat(left_rect, right_rect, stereo_combined);
            cv::imshow("Rectified Stereo Pair", stereo_combined);

           
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