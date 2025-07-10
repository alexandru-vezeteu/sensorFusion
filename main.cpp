#include <iostream>
#include <fstream>
#include <limits>
#include <thread>
#include <chrono>
#include "include/rplidar.h"


#include "include/sl_lidar_driver.h"
using namespace std::chrono_literals;








int main()
{
    


    ///  Create a communication channel instance
    //sl::IChannel* _channel;
    sl::Result<sl::IChannel*> channel = sl::createSerialPortChannel("/dev/ttyUSB0", 115200);
    ///  Create a LIDAR driver instance
    sl::ILidarDriver * lidar = *sl::createLidarDriver();
    auto res = lidar->connect(*channel);
    if(SL_IS_OK(res)){
        sl_lidar_response_device_info_t deviceInfo;
        res = lidar->getDeviceInfo(deviceInfo);
        if(SL_IS_OK(res)){
            printf("Model: %d, Firmware Version: %d.%d, Hardware Version: %d\n",
            deviceInfo.model,
            deviceInfo.firmware_version >> 8, deviceInfo.firmware_version & 0xffu,
            deviceInfo.hardware_version);
        }else{
            fprintf(stderr, "Failed to get device information from LIDAR %08x\r\n", res);
        }
    }else{
        fprintf(stderr, "Failed to connect to LIDAR %08x\r\n", res);
    }
    
    std::vector<sl::LidarScanMode> scanModes;
    lidar->getAllSupportedScanModes(scanModes);

    int iter = 10;
    std::cout<<"MAKE SURE THE ENV IS CLEAR\nHOW MANY ITERS:\n";
    std::cin>>iter;

    float min_distance_target{};
    std::cout<<"CLOSEST OBJ:\n";
    std::cin>>min_distance_target;
    min_distance_target-=1.5;
    for(auto i: scanModes)
    {
        float s{0.0f};
        std::cout<<i.scan_mode<<std::endl;
        std::cout<<"TIME PER SAMPLE:\t"<<i.us_per_sample<<std::endl;
        std::cout<<"MAX DISTANCE:\t\t"<<i.max_distance<<std::endl;
        for(int j=0;j<iter;++j)
        {
            lidar->startScanExpress(false, i.id);
            sl_lidar_response_measurement_node_hq_t nodes[8192];
            size_t size{sizeof(nodes)/sizeof(sl_lidar_response_measurement_node_hq_t)};
            auto res = lidar->grabScanDataHq(nodes, size);
            if(IS_FAIL(res))
            {
                std::cout<<"FAILED"<<std::endl;
                continue;
            }
            else
            {
                lidar->ascendScanData(nodes, size);
                
                float min_d{std::numeric_limits<float>::max()}, max_d{std::numeric_limits<float>::min()};
                float min_angle, max_angle;

                for(auto n:nodes)
                {
                    float angle = n.angle_z_q14 * 90.0f / (1<<14);
                    float distance = static_cast<float>(n.dist_mm_q2) / 10.0f / (1<<2); //cm
                    if(distance<20)
                    {
                        distance*=0.75;
                    }
                    distance;
                    if(distance>6 && distance<1200)
                    {
                        if(distance<min_d)
                        {
                            min_d = distance;
                            min_angle = angle;
                            continue;
                        }

                        if(distance>max_d)
                        {
                            max_d = distance;
                            max_angle = angle;
                            continue;
                        }
                    }
                }
            
                // std::cout<<"MIN DIST:\t"<<min_d<<"\t AT:\t"<<min_angle<<std::endl;
                // std::cout<<"MAX DIST:\t"<<max_d<<"\t AT:\t"<<max_angle<<std::endl;
                // std::cout<<std::endl;
                s+=min_d;
                
                
            }


        }

        auto err = (s/iter-min_distance_target)/min_distance_target;
        std::cout<<"ERROR:\t"<<err*100<<std::endl;
        std::cout<<"MIN MEAN:\t"<<s/iter<<std::endl;
        std::cout<<std::endl;
    }


    lidar->stop();
    delete lidar;
    
}