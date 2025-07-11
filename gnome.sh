sudo apt update
sudo apt install gdm3 gnome-shell gnome-terminal gnome-text-editor
sudo apt install firefox-esr nautilus nautilus-extension-gnome-terminal ffmpegthumbnailer dconf-editor
sudo systemctl enable gdm && sudo systemctl set-default graphical.target 
sudo reboot