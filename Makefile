
display:
	g++ -Wall -c displayLidar.cpp -o display.o
	g++ -Wall display.o ./lib/libsl_lidar_sdk.a -lGL -lGLU -lglut -lGLEW -o display
	rm display.o

lidar:
	g++ -Wall -c lidarTest.cpp -o lidarTest.o
	g++ -Wall lidarTest.o ./lib/libsl_lidar_sdk.a -o lidar
	rm lidarTest.o

camera:
	g++ -std=c++17 cameraTest.cpp -o camera -I/usr/local/include/libcamera -I/usr/include/opencv4 -L/usr/local/lib/aarch64-linux-gnu -lcamera -lcamera-base -lopencv_stitching -lopencv_alphamat -lopencv_aruco -lopencv_barcode -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cvv -lopencv_dnn_objdetect -lopencv_dnn_superres -lopencv_dpm -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_intensity_transform -lopencv_line_descriptor -lopencv_mcc -lopencv_quality -lopencv_rapid -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_shape -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_optflow -lopencv_surface_matching -lopencv_tracking -lopencv_highgui -lopencv_datasets -lopencv_text -lopencv_plot -lopencv_ml -lopencv_videostab -lopencv_videoio -lopencv_viz -lopencv_wechat_qrcode -lopencv_ximgproc -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_dnn -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core -o camera
	
.PHONY: clean
clean:
	rm -f camera
	rm -f lidar
	rm -f display