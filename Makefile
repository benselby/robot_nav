default:
	g++ -o unwrap unwrap.cpp `pkg-config opencv --libs --cflags`
	g++ -o undistort undistort.cpp `pkg-config opencv --libs --cflags`
	g++ -o stereo_disp stereo_vision.cpp `pkg-config opencv --libs --cflags`
	g++ -o stereo_match stereo_match.cpp `pkg-config opencv --libs --cflags`
	
clean:
	rm stereo_disp stereo_match undistort unwrap
