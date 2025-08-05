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
#include <vector>
#include <string>
#include <mutex>
#include <queue>
#include <thread>
const std::vector<std::string> classNames = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};
std::queue<cv::Mat> q;
using namespace libcamera;
using namespace std::chrono_literals;

static std::shared_ptr<Camera> camera;
static unsigned int imageWidth;
static unsigned int imageHeight;
static unsigned int imageStride;

std::mutex mtx;
std::condition_variable cond_var;
bool cond = false;

cv::dnn::Net net;
static bool cameraWindowCreated = false;
static bool detectionWindowCreated = false;
const float CONFIDENCE_THRESHOLD = 0.2f;
const float NMS_THRESHOLD = 0.45f;
    std::vector<int> indices_;
    std::vector<cv::Rect>boxes_;
    std::vector<int> classIds_;
    std::vector<float> confidences_;

using namespace cv;
using namespace std;
using namespace cv::dnn;

    
    void detectObjects(cv::Mat& frame)
    {
    
        cv::Mat blob;
        cv::dnn::blobFromImage(frame, blob, 1.0/255.0, cv::Size(640, 640), cv::Scalar(), true, false, CV_32F);

        net.setInput(blob);

        std::vector<cv::Mat> outputs;
        net.forward(outputs, net.getUnconnectedOutLayersNames());

        cv::Mat detection_output = outputs[0].reshape(1, outputs[0].size[1]);
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
    
        for (int i = 0; i < detection_output.rows; ++i)
        {
            float* data = (float*)detection_output.row(i).data; // Get pointer to the current row

            float x1 = data[0];
            float y1 = data[1];
            float x2 = data[2];
            float y2 = data[3];
            float confidence = data[4];
            int classId = static_cast<int>(data[5]);
            

            if (confidence >= CONFIDENCE_THRESHOLD) {
                float x_factor = static_cast<float>(frame.cols) / 640.0f;
                float y_factor = static_cast<float>(frame.rows) / 640.0f;

                int left = static_cast<int>(x1 * x_factor);
                int top = static_cast<int>(y1 * y_factor);
                int right = static_cast<int>(x2 * x_factor);
                int bottom = static_cast<int>(y2 * y_factor);

                // Create the bounding box rectangle
                int width = std::max(0, right - left);
                int height = std::max(0, bottom - top);

                // Or, more correctly, ensure x1 is always the minimum x, and y1 the minimum y
                int actual_left = std::min(left, right);
                int actual_top = std::min(top, bottom);
                int actual_width = std::abs(right - left);
                int actual_height = std::abs(bottom - top);

                boxes.push_back(cv::Rect(actual_left, actual_top, actual_width, actual_height));
                            confidences.push_back(confidence);
                            classIds.push_back(classId);
                }
            }

            // Apply Non-Maximum Suppression to remove redundant overlapping boxes
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);
        boxes_ = boxes;
        indices_ = indices;
        confidences_ = confidences;
        classIds_ = classIds;
    }

    void addMarkers(cv::Mat& frame)
    {
        for (int idx : indices_) {
        // You can print the index here to confirm detections are being processed
        // std::cout << "Detected idx: " << idx << std::endl;
            cv::Rect box = boxes_[idx];
            int classId = classIds_[idx];
            float confidence = confidences_[idx];

            // Ensure classId is within bounds
            if (classId < 0 || classId >= classNames.size()) {
                std::cerr << "Warning: Invalid class ID " << classId << " detected." << std::endl;
                continue;
            }

            // Draw rectangle
    
            cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2); // Green color, 2px thickness

            // Prepare label text
            std::string label = classNames[classId] + ": " + cv::format("%.2f", confidence);
            std::cout<<label<<std::endl;

            // Get text size for background rectangle
            int baseLine;
            cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

            // // Draw background rectangle for text
            // cv::rectangle(frame, cv::Point(box.x, box.y - labelSize.height - baseLine),
            //               cv::Point(box.x + labelSize.width, box.y),
            //               cv::Scalar(0, 255, 0), cv::FILLED); // Green filled rectangle

            // Put text on the frame
            cv::putText(frame, label, cv::Point(box.x, box.y - baseLine),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1); // Black text
        }
        std::cout<<std::endl<<std::endl;
    }
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
            std::cerr << "mmap failed: " << strerror(errno) << std::endl;
            request->reuse(Request::ReuseBuffers);
            camera->queueRequest(request);
            return;
        }

       
        cv::Mat rgbFrame(imageHeight, imageWidth, CV_8UC3, memory, imageStride);

        q.push(rgbFrame.clone());
        
        munmap(memory, plane.length);
    }

    request->reuse(Request::ReuseBuffers);
    camera->queueRequest(request);
}

void detect()
{
    static int counter{0};
        const float fps = 56.03;
        const int rate = 7;
    while(true)
    {
        if(!q.empty())
        {
            cv::Mat image = q.front();
            q.pop();
            if (!detectionWindowCreated) {
                cv::namedWindow("Detections", cv::WINDOW_AUTOSIZE);
                detectionWindowCreated = true;
            }
            ++counter;
            if(counter>fps/rate)
            {
                counter = 0;
                detectObjects(image);
                std::cout<<"detectie"<<std::endl;
                q = std::queue<cv::Mat>();
                

            }
            addMarkers(image);
            cv::imshow("Detections", image);
            int key = cv::waitKey(1);
            switch(key)
            {
                case 'q':
                case 'Q':
                {
                    std::unique_lock lck{mtx};
                    cond = true;
                    cond_var.notify_one();
                    return;
                    break;
                }
            }
        }
    }
           
}

int main(int argc, char** argv)
{
    if (argc < 4) 
    {
        std::cerr << "Usage: " << argv[0] << " <camera_id_number> <width> <height>" << std::endl;
        std::cerr << "Example: " << argv[0] << " 0 640 480" << std::endl;
        return -1;
    }

    int cameraNumber{};
    int width{};
    int height{};
    
    try
    {
        cameraNumber = std::stoi(argv[1]);
        width = std::stoi(argv[2]);
        height = std::stoi(argv[3]);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        std::cerr << "Usage: " << argv[0] << " <camera_id_number> <width> <height>" << std::endl;
        return -1;
    }

    std::string modelPath = "yolov10n.onnx"; 

    try 
    {
        net = cv::dnn::readNet(modelPath);
        if (net.empty()) {
            std::cerr << "Error: Failed to load DNN model from " << modelPath << std::endl;
            return -1;
        }
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        std::cout << "Successfully loaded YOLO model: " << modelPath << std::endl;
    }
    catch (const cv::Exception& e)
    {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        std::cerr << "Please ensure '" << modelPath << "' exists and is a valid ONNX model." << std::endl;
        return -1;
    }
    
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    int ret = cm->start();
    if (ret) {
        std::cerr << "Failed to start camera manager: " << ret << std::endl;
        return -1;
    }
    
    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::cout << "No cameras were identified on the system." << std::endl;
        cm->stop();
        return EXIT_FAILURE;
    }

    if (cameraNumber < 0 || cameraNumber >= cameras.size()) {
        std::cerr << "Invalid camera ID number. Available cameras: " << cameras.size() << std::endl;
        cm->stop();
        return -1;
    }

    std::string cameraId = cameras[cameraNumber]->id();

    camera = cm->get(cameraId);
    ret = camera->acquire();
    if (ret) {
        std::cerr << "Failed to acquire camera: " << ret << std::endl;
        cm->stop();
        return -1;
    }
    std::cout << "Acquired camera: " << camera->id() << std::endl;

    std::unique_ptr<CameraConfiguration> config = 
        camera->generateConfiguration( { StreamRole::VideoRecording } );
    if (!config) {
        std::cerr << "Failed to generate camera configuration." << std::endl;
        camera->release();
        cm->stop();
        return -1;
    }

    StreamConfiguration &streamConfig = config->at(0);
    config->at(0).pixelFormat = libcamera::formats::RGB888; // Request BGR format for OpenCV
    config->at(0).size.height =  height;
    config->at(0).size.width = width;

    config->validate();
    ret = camera->configure(config.get());
    if (ret) {
        std::cerr << "Failed to configure camera: " << ret << std::endl;
        camera->release();
        cm->stop();
        return -1;
    }
    imageWidth = streamConfig.size.width;
    imageHeight = streamConfig.size.height;
    imageStride = streamConfig.stride;

    std::cout << "Camera configured: " << imageWidth << "x" << imageHeight << ", Stride: " << imageStride << std::endl;

    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

    for (StreamConfiguration &cfg : *config) {
        ret = allocator->allocate(cfg.stream());
        if (ret < 0) {
            std::cerr << "Can't allocate buffers" << std::endl;
            delete allocator;
            camera->release();
            cm->stop();
            return -ENOMEM;
        }
        size_t allocated = allocator->buffers(cfg.stream()).size();
        std::cout << "Allocated " << allocated << " buffers for stream." << std::endl;
    }   
    Stream *stream = streamConfig.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    
    camera->requestCompleted.connect(requestComplete);

    ret = camera->start();
    if (ret) {
        std::cerr << "Failed to start camera: " << ret << std::endl;
        allocator->free(stream);
        delete allocator;
        camera->release();
        cm->stop();
        return -1;
    }
    std::cout << "Camera started. Press 'q' or 'Q' in the video window to quit." << std::endl;

    for (unsigned int i = 0; i < buffers.size(); ++i) {
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            allocator->free(stream);
            delete allocator;
            camera->release();
            cm->stop();
            return -ENOMEM;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request" << std::endl;
            allocator->free(stream);
            delete allocator;
            camera->release();
            cm->stop();
            return ret;
        }

        camera->queueRequest(request.release());
    }
    
    std::jthread detect_t(detect);
    {
        std::unique_lock lck{mtx};
        cond_var.wait(lck, [](){return cond;});
    }

    std::cout << "Stopping camera..." << std::endl;
    camera->stop();
    camera->requestCompleted.disconnect(requestComplete);
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();
    detect_t.request_stop();
    std::cout << "Application exited gracefully." << std::endl;
    return 0;
}
