#utils
echo "Installing utils"
sudo apt install build-essential cmake gparted -y

#clone + make
##https://github.com/Slamtec/rplidar_sdk

#opencv
echo "Installing OpenCV"
sudo apt install libopencv-dev


#libcamera:
echo "Installing libcamera"
sudo apt install ninja-build meson python3-yaml python3-ply python3-jinja2
git clone https://git.libcamera.org/libcamera/libcamera.git
cd libcamera
meson setup build
sudo ninja -C build install


#OpenGL, freeglut, glew
echo "Installing OpenGL, freeglut, glew:"
sudo apt install mesa-utils freeglut3-dev libglew-dev -y