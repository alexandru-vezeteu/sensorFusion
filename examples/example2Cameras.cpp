#include <iomanip>
#include <iostream>
#include <memory>
#include <sys/mman.h>
#include <functional>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/formats.h>


#include <opencv2/opencv.hpp>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <boost/circular_buffer.hpp>

#include <chrono>
using namespace std::chrono_literals;

class VideoHandler
{
    private:
    bool cond;

    size_t width;
    size_t height;
    size_t stride;
    libcamera::PixelFormat pixelFormat;
    
    boost::circular_buffer<cv::Mat> queue;

    std::shared_ptr<libcamera::Camera> camera;


    libcamera::Stream *stream;
    libcamera::FrameBufferAllocator *allocator;
    std::unique_ptr<libcamera::CameraConfiguration> config;

    std::shared_ptr<libcamera::StreamConfiguration> streamConfig;

    public:
    VideoHandler(  std::shared_ptr<libcamera::Camera> camera, 
                    size_t width, size_t height, size_t queue_size = 5,
                    libcamera::PixelFormat pixelFormat = libcamera::formats::RGB888):

                    height{height}, width{width}, camera{camera},
                    cond{true}, pixelFormat{pixelFormat}, 
                    queue{queue_size}, streamConfig{}
    {
        camera->acquire();
    }

    void configure()
    {
        config = 
            camera->generateConfiguration( { libcamera::StreamRole::VideoRecording } );
        streamConfig = std::make_shared<libcamera::StreamConfiguration>(config->at(0));
        
        config->at(0).pixelFormat   =   pixelFormat;
        config->at(0).size.height   =   height;
        config->at(0).size.width    =   width;
        config->validate();
        
        camera->configure(config.get());

        width   = streamConfig->size.width;
        height  = streamConfig->size.height;
        stride  = streamConfig->stride;
    }
    

    void startStreaming()
    {
        allocator = new libcamera::FrameBufferAllocator(camera);
        for (auto &cfg : *config) 
        {
            int ret = allocator->allocate(cfg.stream());
            if (ret < 0) 
            {
                std::cerr << "Can't allocate buffers" << std::endl;
                exit(-ENOMEM);
            }
            size_t allocated = allocator->buffers(cfg.stream()).size();
        }
        stream = streamConfig->stream();
        const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = allocator->buffers(stream);
        std::vector<std::unique_ptr<libcamera::Request>> requests;
        for (unsigned int i = 0; i < buffers.size(); ++i) {
            std::unique_ptr<libcamera::Request> request = camera->createRequest();
            if (!request)
            {
                std::cerr << "Can't create request" << std::endl;
                exit(-ENOMEM);
            }

            const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[i];
            int ret = request->addBuffer(stream, buffer.get());
            if (ret < 0)
            {
                std::cerr << "Can't set buffer for request"
                    << std::endl;
                exit(ret);
            }

            requests.push_back(std::move(request));
        }
        camera->requestCompleted.connect(this, &VideoHandler::requestComplete);

        
        camera->start();

        for (auto &request : requests)
            camera->queueRequest(request.get());
    }

    ~VideoHandler()
    {
        camera->stop();
        allocator->free(stream);
        delete allocator;
        camera->release();
        camera.reset();
    }


    // VideoHandler(const VideoHandler& vc) = delete;
    // Video



    boost::circular_buffer<cv::Mat>& getQueue()
    {
        return queue;
    }

    void requestStop()
    {
        cond = false;
    }
    private:
    void requestComplete(libcamera::Request* request)
    {
        std::cout<<"SJKSDKSD\n";
        if(!cond)
            return;

        if (request->status() == libcamera::Request::RequestCancelled)
            return;

        const std::map<const libcamera::Stream *, libcamera::FrameBuffer *> &buffers = request->buffers();
        for (auto bufferPair : buffers) {
            libcamera::FrameBuffer *buffer = bufferPair.second;

            const libcamera::FrameBuffer::Plane &plane = buffer->planes()[0];
            int fd = plane.fd.get();

            void *memory = mmap(NULL, plane.length, PROT_READ, MAP_SHARED, fd, 0);
            if (memory == MAP_FAILED) {
                std::cerr << "mmap failed" << std::endl;
                return;
            }


            cv::Mat rgbFrame(height, width, CV_8UC3, memory, stride);
            queue.push_back(rgbFrame);
            std::cout<<camera->id()<<"\n";

            munmap(memory, plane.length);
        }

        request->reuse(libcamera::Request::ReuseBuffers);
        camera->queueRequest(request);
    }
};

std::condition_variable cond_var;
std::mutex mtx;
bool stop = false;

void display2Cameras(   boost::circular_buffer<cv::Mat>& q1, 
                        boost::circular_buffer<cv::Mat>& q2
                    )
{
    cv::Mat m1{};
    cv::Mat m2{};
    cv::namedWindow("Display 2 cameras", cv::WINDOW_NORMAL);
    while(true)
    {  
        if(!q1.empty())
        {
            m1 = q1.front();
            q1.pop_front();
        }
        if(!q2.empty())
        {
            m2 = q2.front();
            q2.pop_front();
        }
        

        cv::Mat disp;
        cv::hconcat(m1, m2, disp);
        if(!disp.empty())
        {
            
            cv::imshow("Display 2 cameras", disp);
            int key = cv::waitKey(1);
            switch(key)
            {
                case 27:
                case 'q':
                case 'Q':
                {
                    std::unique_lock lck{mtx};
                    stop = true;
                    cond_var.notify_one();
                    return;
                }
                break;
            }
        }
    }
}


int main(int argc, char** argv)
{
     if (argc < 7) 
    {
        std::cerr << "Usage: " << argv[0] << " id width height" << std::endl;
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
        std::cerr << "Usage: " << argv[0] << " id width height" << std::endl;
        return -1;
    }


    std::unique_ptr<libcamera::CameraManager> cm = std::make_unique<libcamera::CameraManager>();
    cm->start();


    {
        auto c = cm->cameras()[cameraNumber1];
        VideoHandler cam1{c, width1, height1};

        c = cm->cameras()[cameraNumber2];
        VideoHandler cam2{c, width2, height2};

        cam1.configure();
        cam2.configure();

        cam1.startStreaming();
        cam2.startStreaming();
                


        //std::thread thread{display2Cameras, std::ref(cam1.getQueue()), std::ref(cam2.getQueue())};
        std::this_thread::sleep_for(5s);
        // std::unique_lock lck(mtx);
        // cond_var.wait(lck, [](){return stop;});

    }
    




    cm->stop();
}