/*
*  A sandbox program for experimenting with OpenCV's stereo vision functions
*  for use with the telepresence robot. 
*
*  Ben Selby, September 2013
*/

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <string>

const std::string output_path = "stereo_output/";

int get_time_diff( struct timeval *result, struct timeval *t1, struct timeval *t2 )
{
    long int diff = (t2->tv_usec + 1000000*t2->tv_sec) - (t1->tv_usec + 1000000*t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
        
    return (diff>0);
}

int main( int argc, char** argv )
{
	bool save = false;	
	
	if ( argc < 3 ) 
    {
        std::cout<< "Usage: "<<argv[0]<<" <top image> <bottom image> [optional: -save]" << std::endl;
        return -1;
    }
    
    if ( argc > 3 )
    {
	    // Check for the "save image flag"
    	if ( strcmp( "-s", argv[3] ) == 0 || strcmp( "-save", argv[3] ) == 0 )
    		save = true; 
    } 
    
    cv::Mat top_img = cv::imread( argv[1], 0 );
    cv::Mat bottom_img = cv::imread( argv[2], 0 );
    
    if ( ! top_img.data || ! bottom_img.data )
    {
    	std::cout<<"Failed to load one of the images, exiting." << std::endl;
    	return -1;
    }
    
	struct timeval start_time, end_time, time_diff, calc_time;
	gettimeofday(&start_time, NULL);
	
	cv::Mat disparity, disp8;
	cv::StereoBM bm_state = cv::StereoBM(CV_STEREO_BM_BASIC, 32, 5);
	bm_state( top_img, bottom_img, disparity );
	disparity.convertTo( disp8, CV_8U );
	imshow( "Disparity", disp8 );
				
//	// Create the stereoBM state:
//	CvStereoBMState* bm_state = cvCreateStereoBMState();
//	
//	// Create a matrix to store the disparity
//	cv::Mat disparity;
//	disparity.create( top_img.size(), top_img.type() );
//	
//	// Find the correspondence:
//	cv::cvFindStereoCorrespondenceBM( top_img, bottom_img, disparity, bm_state );
	
	cv::waitKey(0);
	return 0;
}
