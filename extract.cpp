/*
*  A simple openCV program to extract the first frame from a video
*
*  Ben Selby, November 2013
*/

#include <opencv2/highgui/highgui.hpp>
#include <iostream>

int main( int argc, char** argv )
{
	if ( argc < 3 )
	{
		std::cout<<"Usage: "<<argv[0] <<" <infile> <outfile>"<<std::endl;
		return -1;
	}
	
	std::cout<<"Capturing video from '"<<argv[1]<<"'...";
	cv::VideoCapture capture( argv[1] );
	std::cout<<" done." <<std::endl;
		
	// Check if the capture object successfully initialized
	if ( !capture.isOpened() ) 
	{
		std::cout<< "Failed to load video, exiting." <<std::endl;
		return -1;
	}
	
	cv::Mat frame;
	capture.read( frame );
	imshow("Frame Grabbed", frame);	 
	std::cout<< "Press 's' to save the image." <<std::endl;
	
	char saved = cv::waitKey(0);
	
	// if 's' key is pressed, save the unwrapped image
	if ( saved == 's' || saved == 'S' )
	{	
		imwrite(argv[2], frame);		
	}
	
	return 0;
}
