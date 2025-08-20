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
#include <condition_variable>
#include <filesystem>

using namespace libcamera;
using namespace std::chrono_literals;
static std::shared_ptr<Camera> camera;
static unsigned int imageWidth;
static unsigned int imageHeight;
static unsigned int imageStride;

std::mutex mtx;
std::condition_variable cond_var;
bool cond = false;

static void requestComplete(Request *request)
{
    static int imageCount = 0;
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

        
        unsigned int width = imageWidth;
        unsigned int height = imageHeight;
        unsigned int stride = imageStride;
        
        static bool windowCreated = false;
        if (!windowCreated) {
            cv::namedWindow("Camera", cv::WINDOW_NORMAL);
            
            windowCreated = true;
        }

        cv::Mat rgbFrame(height, width, CV_8UC3, memory, stride);

        
        
        
        cv::imshow("Camera", rgbFrame);
        
        int key = cv::waitKey(1);
        switch(key)
        {
            case 'c':
            {
                std::string filename = "pics/saved_image_" + std::to_string(imageCount++) + ".png";
                cv::imwrite(filename, rgbFrame);
                std::cout << "Image saved as: " << filename << std::endl;
            } break;
            case 'q':
            {
                std::unique_lock lck{mtx};
                cond = true;
                cond_var.notify_one();
                munmap(memory, plane.length);
                return;
            } break;
        }
        

        munmap(memory, plane.length);
    }

    request->reuse(Request::ReuseBuffers);
    camera->queueRequest(request);
}


int main(int argc, char** argv)
{
    if (argc < 4) 
    {
        std::cerr << "Usage: " << argv[0] << " id width height" << std::endl;
        return -1;
    }

    int cameraNumber{}, width{}, height{};
    
    try
    {
        cameraNumber = std::stoi(argv[1]);
        width = std::stoi(argv[2]);
        height = std::stof(argv[3]);
    }
    catch(...)
    {
        std::cerr << "Usage: " << argv[0] << " id width height" << std::endl;
        return -1;
    }
    

    std::filesystem::path pics_dir = "pics";
    try {
        if (!std::filesystem::exists(pics_dir)) {
            std::filesystem::create_directory(pics_dir);
            std::cout << "Created directory: " << pics_dir << std::endl;
        } else {
            std::cout << "Directory already exists: " << pics_dir << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating directory " << pics_dir << ": " << e.what() << std::endl;
        return -1;
    }

    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();
    
    
    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::cout << "No cameras were identified on the system."
                << std::endl;
        cm->stop();
        return EXIT_FAILURE;
    }

    std::string cameraId = cameras[cameraNumber]->id();

    camera = cm->get(cameraId);
    camera->acquire();
    std::unique_ptr<CameraConfiguration> config = 
        camera->generateConfiguration( { StreamRole::VideoRecording } );
    StreamConfiguration &streamConfig = config->at(0);
    config->at(0).pixelFormat = libcamera::PixelFormat(libcamera::formats::RGB888);
    config->at(0).size.height =  height;
    config->at(0).size.width = width;

    config->validate();
    camera->configure(config.get());
    imageWidth = streamConfig.size.width;
    imageHeight = streamConfig.size.height;
    imageStride = streamConfig.stride;

    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

    for (StreamConfiguration &cfg : *config) {
        int ret = allocator->allocate(cfg.stream());
        if (ret < 0) {
            std::cerr << "Can't allocate buffers" << std::endl;
            return -ENOMEM;
        }
        size_t allocated = allocator->buffers(cfg.stream()).size();
    }   
    Stream *stream = streamConfig.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    std::vector<std::unique_ptr<Request>> requests;
    for (unsigned int i = 0; i < buffers.size(); ++i) {
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            return -ENOMEM;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request"
                << std::endl;
            return ret;
        }

        requests.push_back(std::move(request));
    }
    camera->requestCompleted.connect(requestComplete);

    camera->start();
    for (std::unique_ptr<Request> &request : requests)
        camera->queueRequest(request.get());

    std::cout<<std::endl<<std::endl<<std::endl;
    std::cout<<"Click on the video window and press c to capture pic and q to exit."<<std::endl;
    std::unique_lock lck{mtx};
    cond_var.wait(lck, [](){return cond;});

    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();
    return 0;
}