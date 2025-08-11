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

bool cond = false;
std::condition_variable cond_var;
std::mutex mtx;

std::shared_mutex mtx1, mtx2;

static void requestComplete1(Request *request)
{
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
    cv::Mat m1, m2;
    int imageCount{0};
    cv::namedWindow("Display 2 cameras", cv::WINDOW_NORMAL);
    while(true)
    {  
        {
            if(!q1.empty())
            {
                {
                    std::shared_lock lck{mtx1};
                    m1 = q1.front();
                }
                std::unique_lock lck{mtx1};
                q1.pop_front();
            }
        }
        {
            if(!q2.empty())
            {
                {
                    std::shared_lock lck{mtx2};
                    m2 = q2.front();
                }
                std::unique_lock lck{mtx2};
                q2.pop_front();
            }
        }
        

        cv::Mat disp;
        if (!m1.empty() && !m2.empty())
        {
            int height1 = m1.rows;
            int height2 = m2.rows;

            int maxHeight = std::max(height1, height2);

            cv::Mat m1_padded, m2_padded;

            if (height1 < maxHeight) {
                int padding = maxHeight - height1;
                cv::copyMakeBorder(m1, m1_padded, 0, padding, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0)); // pad bottom
            } else {
                m1_padded = m1;
            }

            if (height2 < maxHeight) {
                int padding = maxHeight - height2;
                cv::copyMakeBorder(m2, m2_padded, 0, padding, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0)); // pad bottom
            } else {
                m2_padded = m2;
            }

            cv::hconcat(m1_padded, m2_padded, disp);
            cv::imshow("Display 2 cameras", disp);
        }

        else if (!m1.empty())
        {
            cv::imshow("Display 2 cameras", m1);
        }
        else if (!m2.empty())
        {
            cv::imshow("Display 2 cameras", m2);
        }
        
        int key = cv::waitKey(1);
        switch(key)
        {
            case 27:
            case 'q':
            case 'Q':
            {
                std::unique_lock lck{mtx};
                cond_var.notify_all();
                cond = true;
                cv::destroyAllWindows();
                return;
            }
            break;

            case 'c':
            case 'C':
            {
                std::string filename = "pics/left/saved_image_" + std::to_string(imageCount) + ".png";
                cv::imwrite(filename, m1);
                std::cout << "Image saved as: " << filename << std::endl;

                filename = "pics/right/saved_image_" + std::to_string(imageCount++) + ".png";
                cv::imwrite(filename, m2);
                std::cout << "Image saved as: " << filename << std::endl;
                
            }
            break;
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
    {
        std::unique_lock lck(mtx);
        cond_var.wait(lck, [](){return cond;});
    }
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