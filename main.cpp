#include <iostream>
#include <fstream>
#include <limits>
#include "include/rplidar.h"
#include <string.h>
#include <math.h>
#include "include/sl_lidar_driver.h"
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
    std::cout<<"POT SCANA:"<<std::endl;
    for(auto i: scanModes)
    {
        std::cout<<"TIME PER SAMPLE:\t"<<i.us_per_sample<<std::endl;
        std::cout<<"MAX DISTANCE:\t\t"<<i.max_distance<<std::endl;
        std::cout<<i.scan_mode<<std::endl;
        lidar->startScanExpress(false, i.id);
        sl_lidar_response_measurement_node_hq_t nodes[8192];
        size_t size{sizeof(nodes)/sizeof(sl_lidar_response_measurement_node_hq_t)};
        auto res = lidar->grabScanDataHq(nodes, size);
        char* bitmap = new char[1024*1024];
        int centerX = 512, centerY = 512;
        float unit = 12.0f/1024;
        memset(bitmap, std::numeric_limits<char>::max(),  1024*1024*sizeof(char));
        if(IS_FAIL(res))
        {
            std::cout<<"FAILED"<<std::endl;
            continue;
        }
        else
        {
            lidar->ascendScanData(nodes, size);
            int avg{0};
            float avg_d{0.0f};
            int m{0};
            for(auto n:nodes)
            {
                float angle = n.angle_z_q14 * 90.0f / (1<<14);
                float distance = static_cast<float>(n.dist_mm_q2) / 1000.0f / (1<<2);
                // std::cout<<"UNGHI:\t\t"<<angle<<std::endl;
                //std::cout<<"DISTANCE:\t"<<distance<<std::endl;
                // std::cout<<"QUALITY:\t\t"<<static_cast<int>(n.quality)<<std::endl;
                avg+=n.quality;
                if(distance<16 && distance>0)
                {
                    avg_d+=distance;
                    m+=1;
                    int posX = distance/unit, posY = distance/unit;
                    float s = sin(angle*M_PI/180);
                    float c = cos(angle*M_PI/180);
                    int newX = posX*c-posY*s+centerX;
                    int newY = posX*s+posY*c+centerY;

                    if(1024*newY+newX>-1 && 1024*newY+newX<1024*1024)
                        bitmap[1024*newY+newX] = 0;
                }
                //std::cout<<distance<<" ";
                //std::cout<<std::endl;
            }
           
            std::cout<<"AVG QUALITY:\t"<<static_cast<float>(avg)/size/
                                        std::numeric_limits<_u8>::max()<<std::endl;
            std::cout<<"AVG D:\t"<<avg_d/m<<std::endl<<std::endl;
            
            
            std::ofstream file("ok");
            file<<bitmap;
            file.close();

            delete bitmap;
            break;
        }

    }


    lidar->stop();
    delete lidar;
    
}