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
#include <mutex>

using namespace libcamera;
using namespace std::chrono_literals;
static std::shared_ptr<Camera> camera;
static unsigned int imageWidth;
static unsigned int imageHeight;
static unsigned int imageStride;

bool cond = false;
std::condition_variable cond_var;
std::mutex mtx;

static void requestComplete(Request *request)
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

        unsigned int width = imageWidth;
        unsigned int height = imageHeight;
        unsigned int stride = imageStride;

        cv::Mat rgbFrame(height, width, CV_8UC3, memory, stride);

       
        cv::namedWindow("Camera", cv::WINDOW_NORMAL);
       
        cv::imshow("Camera", rgbFrame);
        int key = cv::waitKey(1);
        switch(key)
        {
            case 'Q':
            case 'q':
            {
                std::unique_lock lck{mtx};
                cond = true;
                cv::destroyAllWindows();
                munmap(memory, plane.length);
                cond_var.notify_one();
                return;
            }
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

    // Code to follow
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();
    for (auto const &camera : cm->cameras())
        std::cout << camera->id() << std::endl;
    
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
    config->at(0).size.height = height;
    config->at(0).size.width = width;
    camera->configure(config.get());
    std::cout << "Pixel format used: " << streamConfig.pixelFormat.toString() << std::endl;
    imageWidth = streamConfig.size.width;
    imageHeight = streamConfig.size.height;
    imageStride = streamConfig.stride;
    std::cout<<"Res: "<< imageHeight<<"x"<<imageWidth<<std::endl;

    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

    for (StreamConfiguration &cfg : *config) {
        int ret = allocator->allocate(cfg.stream());
        if (ret < 0) {
            std::cerr << "Can't allocate buffers" << std::endl;
            return -ENOMEM;
        }

        size_t allocated = allocator->buffers(cfg.stream()).size();
        //std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
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


    {
        std::unique_lock lck(mtx);
        cond_var.wait(lck, [](){return cond;});
    }

    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();
    return 0;
}