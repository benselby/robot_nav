/**
*  This program takes a single image from the a camera pointed at a 
*  spherical mirror and unwraps it to produce a 360-degree panoramic view.
*
*  This is to be used for tracking of people and navigation of a robot. 
*
*  Ben Selby, 2013 
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
	if ( argc != 2 ) 
    {
        printf( "Usage: %s <image_filename> \n", argv[0] );
        return -1;
    }
	
	cv::Mat src, cropped_img, unwrapped_img;
	cv::Mat map_x, map_y;
	
	struct timeval start_time, end_time, time_diff, calc_time;
	gettimeofday(&start_time, NULL);
	
	// Load the specified image
	std::string path = argv[1];
	src = cv::imread( path, 1 );
	if ( !src.data ) 
	{
		printf( "Failed to load image, exiting.\n" );
		return -1;
	}
	
	// Crop the input image to contain only the mirror
	struct CvSize src_size;
	src_size = src.size();	
	cv::Rect ROI( OFFSET_X, OFFSET_Y, WIDTH, HEIGHT );
	cropped_img = src( ROI );
	
	// create the unwrapped image and maps with same size as the cropped image
	unwrapped_img.create( RADIUS, (int) 2*PI*RADIUS, cropped_img.type() );
	map_x.create( unwrapped_img.size(), CV_32FC1 );
	map_y.create( unwrapped_img.size(), CV_32FC1 );
	
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
    cvNamedWindow("Cropped", 1 );
    imshow( "Cropped", cropped_img );
    cvNamedWindow( "Unwrapped", 1 );
    imshow( "Unwrapped", unwrapped_img );
	
	int saved = cv::waitKey(0);
	
	// if 's' key is pressed, save the unwrapped image
	if ( saved == 1179731 || saved == 1048691 )
	{	
	    int last_index = path.find_last_of("/");
    	std::string img_name = path.substr(last_index+1, path.length()-1);
		std::string prefix = "unwrapped_";
		img_name = prefix + img_name; 
		std::string out_path = output_path + img_name;
		std::cout<<"Saved unwrapped image to: "<<out_path<<std::endl;
		imwrite(out_path, unwrapped_img);		
	}
	return 0;
}
