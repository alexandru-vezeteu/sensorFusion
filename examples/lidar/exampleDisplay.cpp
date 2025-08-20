#define SCALE 10  // Distance scale for screen
#define SCREENX 1024 // Size of Display window
#define SCREENY 1024
#define STARTX 100  // Coordinates to place window
#define STARTY 100

//mesa-utils freeglut3-dev libglew-dev

// Import Headers
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>
#include "rplidar.h"
#include "sl_lidar_driver.h"


class color {
public:
    float r;
    float g;
    float b;
    color(float rV=0.0, float gV=0.0, float bV=0.0);
};

color::color( float rV, float gV, float bV)
{
    r = rV;
    g = gV;
    b = bV;
}
color RED(1.0,0.0,0.0);
color GREEN(0.0,1.0,0.0);
color BLUE(0.0,0.0,1.0);

void point_polar(float theta, float r, int offsetx, int offsety, float size, color c)
{
    int x = r * cos(theta) + offsetx;
    int y = r * sin(theta) + offsety;

    //std::cout<<x<<" "<<y<<std::endl;
    glPointSize(size);
    glColor3f(c.r, c.g, c.b);
    glBegin(GL_POINTS);
    glVertex2f(x,y);
    glEnd();
    // printf("point_polar: th=%f r=%f, %d, %d\n", theta, r, x, y);   
}



#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif


#include <unistd.h>

static inline void delay(_word_size_t ms){
    while (ms>=1000){
        usleep(1000*1000);
        ms-=1000;
    };
    if (ms!=0)
        usleep(ms*1000);
}







sl::ILidarDriver * drv;
const char * opt_com_path = NULL;
_u32         defbaudrateArray[2] = {115200, 256000};
_u32        *baudrateArray = defbaudrateArray;
_u32         opt_com_baudrate = 0;
u_result     op_result;
int id;

// RENDER - Pull data from LIDAR and render to display
void renderScreen(void){
    std::cout<<"OK"<<std::endl;
    glClear (GL_COLOR_BUFFER_BIT);
    float theta = 0.0;
    float dist = 0.1;
    int quality = 0;

    // Fetch data from LIDAR
    rplidar_response_measurement_node_hq_t nodes[8192];
    size_t   count = _countof(nodes);
    drv->startScanExpress(false, id);
    op_result = drv->grabScanDataHq(nodes, count);
    if (SL_IS_OK(op_result)) {
        drv->ascendScanData(nodes, count);
        std::cout<<count<<std::endl;
        for (int pos = 0; pos < (int)count ; ++pos) {
            theta = (360-((nodes[pos].angle_z_q14 * 90.f / (1 << 14)))/360)*2*M_PI;
            dist = (nodes[pos].dist_mm_q2/4.0f);
            if(dist>12000)
            {
                continue;
            }
            if(dist ==0)
            {
                continue;
            }
            //std::cout<<dist<<" "<<theta<<", ";
            dist = dist/SCALE;
            quality = nodes[pos].quality;
            // Display
            if(quality == 0) {
                point_polar(theta, dist, SCREENX/2, SCREENY/2, 4*3.0f, RED);
            }
            else {
                point_polar(theta, dist, SCREENX/2, SCREENY/2, 4*1.5f, BLUE);
            }
            // printf("\r > Data: %f %f %d      ",theta,dist,quality);
        }
        
    }
    else
    {
        std::cout<<"NOT OK"<<std::endl;
    }
    // Render
    glFlush();
    glutSwapBuffers();

    
}

// MAIN
int main(int argc, char** argv) {


    // Initialize OpenGL and display
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE |GLUT_RGB);
    glutInitWindowSize (SCREENX,SCREENY);
    glutInitWindowPosition (STARTX, STARTY);
    glutCreateWindow ("OpenGL LIDAR Display");
    glewInit();

    glClearColor (1.0, 1.0, 1.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, SCREENX, 0, SCREENY);


    glClear (GL_COLOR_BUFFER_BIT);
    glutDisplayFunc(renderScreen);
    glutIdleFunc(renderScreen);

    printf("LIDAR OpenGL Display for Slamtec RPLIDAR Device\n"
           "Using RPLIDAR SDK Version: " RPLIDAR_SDK_VERSION "\n");

    // Optional - read serial port from the command line...
    

   

    // Create the RPLIDAR SDK driver instance
    drv = *sl::createLidarDriver();
    if (!drv) {
        printf("insufficent memory, exit\n");
        exit(2);
    }
    
   
    sl::Result<sl::IChannel*> channel = sl::createSerialPortChannel("/dev/ttyUSB0", 115200);
    auto res = drv->connect(*channel);

    std::vector<sl::LidarScanMode> scanModes;
    drv->getAllSupportedScanModes(scanModes);
    id = scanModes[1].id;

   
    

   
    glutMainLoop();


    drv->stop();
    delete drv;
    
    

    return 0;
}