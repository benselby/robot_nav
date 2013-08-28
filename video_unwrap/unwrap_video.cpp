/*
*  A program which unwraps video taken from the VirtualME robot for vision
*  processing. 
*  
*  Ben Selby, August 2013
*/

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <string>

#define PI 3.141592654

// define the image parameters for cropping the mirror - should be a square
const int OFFSET_X = 96;
const int OFFSET_Y = 8;
const int WIDTH = 465;
const int HEIGHT = WIDTH;
const int RADIUS = WIDTH/2;

int get_time_diff( struct timeval *result, struct timeval *t1, struct timeval *t2 )
{
    long int diff = (t2->tv_usec + 1000000*t2->tv_sec) - (t1->tv_usec + 1000000*t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
        
    return (diff>0);
}

int main( int argc, char** argv )
{
	if ( argc != 2 ) 
    {
        printf( "Usage: %s <video_filename> \n", argv[0] );
        return -1;
    }
    
	struct timeval start_time, end_time, time_diff, calc_time;
	gettimeofday(&start_time, NULL);
	cvNamedWindow("window", 1);
	
	printf("Capturing video: %s...", argv[1] );
	cv::VideoCapture capture( argv[1] );
	printf(" done.\n");	
	// Load the specified video
	if ( !capture.isOpened() ) 
	{
		printf( "Failed to load video, exiting.\n" );
		return -1;
	}
	
	cv::Mat frame, cropped_img, unwrapped_img;
	cv::Mat map_x, map_y;
//	cv::Rect ROI( OFFSET_X, OFFSET_Y, WIDTH, HEIGHT );
//	cropped_img = frame( ROI );
//	
//	// create the unwrapped image and maps with same size as the cropped image
//	unwrapped_img.create( RADIUS, (int) 2*PI*RADIUS, cropped_img.type() );
//	map_x.create( unwrapped_img.size(), CV_32FC1 );
//	map_y.create( unwrapped_img.size(), CV_32FC1 );
//	
//	// develop the map arrays for the unwarping from polar coordinates
//	// i = y coord, j = x coord	
//	int rows = unwrapped_img.rows;
//	int cols = unwrapped_img.cols;
//	
//    for ( int i = 0; i < rows; i++ )
//    {
//        for ( int j = 0; j < cols; j++ )
//        {
//        	double theta = (double) j / RADIUS;// - (3*PI/2); // discretization in radians
//        	map_x.at<float>(rows-i,j) = RADIUS + i*sin(theta);
//        	map_y.at<float>(rows-i,j) = RADIUS + i*cos(theta);
//        }
//    }
    	
	int i = 0;
	while(true)
	{	
		printf("In the loop :%d\n", i++);
		if ( !capture.read(frame) )
		{
			printf("Failed to read frame, exiting\n");
			break;
		}

		imshow("window", frame);
		char  key = cvWaitKey(10);
		if ( key == 27 ) // ESC 
			break;		
	}
	
	cv::waitKey(0);
	return 0;
}
