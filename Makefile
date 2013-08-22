default:
	g++ -o unwrap unwrap.cpp `pkg-config opencv --libs --cflags`
	g++ -o undistort undistort.cpp `pkg-config opencv --libs --cflags`
