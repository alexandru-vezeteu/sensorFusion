#!/bin/bash
if [ `whoami` != 'root' ]
  then
    echo "You must be root to do this."
    echo "Use: sudo ./req.sh"
    exit
fi


#utils
echo "Installing utils"
apt install build-essential cmake gparted -y
echo 

#clone + make
##https://github.com/Slamtec/rplidar_sdk

#opencv
echo "Installing OpenCV"
apt install libopencv-dev -y
echo


#libcamera:
echo "Installing libcamera"
apt install libcamera-dev rpicam-apps -y


#OpenGL, freeglut, glew
echo "Installing OpenGL, freeglut, glew:"
apt install mesa-utils freeglut3-dev libglew-dev -y
echo

#Docker
echo "Installing docker"
apt-get install ca-certificates curl -y
install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/debian/gpg -o /etc/apt/keyrings/docker.asc
chmod a+r /etc/apt/keyrings/docker.asc
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/debian \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
apt-get update
apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin -y
usermod -aG docker $SUDO_USER


reboot