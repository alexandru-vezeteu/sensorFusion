#!/bin/bash

echo "--- Starting cameraTest container ---"
echo "This script will check for camera devices and run a basic libcamera test."
echo "If you want an interactive shell, run: docker-compose run --rm cameraTest bash"
echo ""

# Check for video devices in /dev
echo "1. Checking for video devices in /dev:"
ls -l /dev/video*
echo ""

# Use v4l2-ctl to list detected devices
echo "2. Running v4l2-ctl --list-devices:"
v4l2-ctl --list-devices
echo ""

# Use libcamera-still to list available cameras
echo "3. Running libcamera-still --list-cameras:"
libcamera-still --list-cameras
echo ""

# --- Run a basic libcamera test ---
# If the first argument to the script is "bash", drop into a bash shell for debugging.
# Otherwise, try to run libcamera-hello for 5 seconds to see if the camera streams.
if [ "$1" = "bash" ]; then
    echo "Dropping into bash shell for interactive debugging."
    exec "/bin/bash"
else
    echo "4. Attempting to run libcamera-hello for 5 seconds (Ctrl+C to stop if interactive):"
    # --info-text shows useful debug info like FPS, exposure, gain
    # -t 5000 runs for 5000 milliseconds (5 seconds)
    libcamera-hello --info-text "%fps %exp %gain" -t 5000
    echo "libcamera-hello finished."
    echo ""
    # If no specific command is given, keep the container running for inspection
    # This allows `docker-compose up` to keep the container alive after the entrypoint script finishes.
    exec "$@"
fi

echo "--- Container finished ---"
