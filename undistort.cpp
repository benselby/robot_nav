/*
* This program is a sandbox for testing methods to undistort (unwarp) 
* the unwrapped image produced by the unwrapping program. 
*
* Currently it is written to only work with a computer-generated image which
* contains a series of horizontal stripes, as will the calibration rig. 
*
* Ben Selby, August 2013
*/ 

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <string>

#define PI 3.141592654

// define the parameters for the output image
const int WIDTH = 465;
const int HEIGHT = WIDTH;

// specify input and output locations:
const std::string output_path = "output/";
const std::string input_path = "input_img/";

int get_time_diff( struct timeval *result, struct timeval *t1, struct timeval *t2 )
{
    long int diff = (t2->tv_usec + 1000000*t2->tv_sec) - (t1->tv_usec + 1000000*t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
        
    return (diff>0);
}

int main( int argc, char** argv )
{
	int num_stripes = 0;
	
	if ( argc < 2 ) 
    {
        printf( "Usage: %s <image_filename> <number of stripes>\n", argv[0] );
        return -1;
    }
    
    if ( argc < 3 )
    {
    	num_stripes = 7;
    }
	else
	{
		num_stripes = argv[2];
	}
	
	cv::Mat src, undistorted_img;
	cv::Mat map_x, map_y;
	
	struct timeval start_time, end_time, time_diff, calc_time;
	gettimeofday(&start_time, NULL);
	
	// Load the specified source image
	std::string path = argv[1];
	src = cv::imread( path, 1 );
	if ( !src.data ) 
	{
		printf( "Failed to load image, exiting.\n" );
		return -1;
	}

	undistorted_img.create( HEIGHT, WIDTH, src.type() );
	map_x.create( undistorted_img.size(), CV_32FC1 );
	map_y.create( undistorted_img.size(), CV_32FC1 );
	
	// develop the map arrays for the unwarping from polar coordinates
	// i = y coord, j = x coord	
	int rows = unwrapped_img.rows;
	int cols = unwrapped_img.cols;
	
    for ( int i = 0; i < rows; i++ )
    {
        for ( int j = 0; j < cols; j++ )
        {
        	double theta = (double) j / RADIUS;// - (3*PI/2); // discretization in radians
        	map_x.at<float>(rows-i,j) = RADIUS + i*sin(theta);
        	map_y.at<float>(rows-i,j) = RADIUS + i*cos(theta);
        }
    }
	
	gettimeofday( &calc_time, NULL);
	// let OpenCV handle the interpolation for the gaps in the unwrapped image
    cv::remap( cropped_img, unwrapped_img, 
    		 map_x,
    		 map_y,
    		 CV_INTER_LINEAR,
    		 cv::BORDER_CONSTANT,
    		 cv::Scalar(0,0,0)
		   );
   
	gettimeofday( &end_time, NULL );
	get_time_diff( &time_diff, &start_time, &end_time );
    printf( "Time Elapsed: %ld.%06ld seconds\n", time_diff.tv_sec, time_diff.tv_usec );
    
    get_time_diff( &time_diff, &calc_time, &end_time );
    printf( "Remap time: %ld.%06ld seconds\n", time_diff.tv_sec, time_diff.tv_usec );
    
    cvNamedWindow( "Original", 1 );
    imshow( "Original", src );
    cvNamedWindow( "undistorted", 1 );
    imshow( "undistorted", undistorted_img );
	
	int saved = cv::waitKey(0);
	
	// if 's' key is pressed, save the unwrapped image
	if ( saved == 1179731 || saved == 1048691 )
	{	
	    int last_index = path.find_last_of("/");
    	std::string img_name = path.substr(last_index+1, path.length()-1);
		std::string prefix = "undistorted_";
		img_name = prefix + img_name; 
		std::string out_path = output_path + img_name;
		std::cout<<"Saved unwrapped image to: "<<out_path<<std::endl;
		imwrite(out_path, undistorted_img);		
	}
	return 0;
}
