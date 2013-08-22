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
#include <fstream>
#include <stdio.h>
#include <sys/time.h>
#include <string>

#define PI 3.141592654

// define the parameters for the output images
const int WIDTH = 1425;
const int HEIGHT = 116;

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

	if ( argc < 4 ) 
    {
        printf( "Usage: %s <image_filename> <cal_input_data.txt> <number_of_lines>\n", argv[0] );
        return -1;
    }    
    
    // Read the pixel values of the lines from the text file and store them in
    // the array y_vals:
    int num_lines = atoi( argv[3] );
    printf("Number of lines: %d\n", num_lines);
    std::ifstream input_data( argv[2] );
    std::string line;
    int y_vals[num_lines*2];
    
    if ( input_data.is_open() )
    {
    	int index = 0;
    	while ( input_data.good() )
    	{
    		getline( input_data, line );
    		y_vals[index] = atoi( line.c_str() );
    		printf("Line %d: %d\n", index, y_vals[index] );
    		index++;    		
    	}
    }
    else
    {
    	printf( "Unable to open the file \"%s\" - exiting.\n", argv[2] ); 
    	return -1;
    }
    
    input_data.close();
    
	printf("Successfully read the calibration file.\n");
	cv::Mat src, top_img, bottom_img;
	cv::Mat top_raw, bottom_raw; // for storing the top and bottom of the src
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

	top_img.create( HEIGHT, WIDTH, src.type() );
	bottom_img.create( HEIGHT, WIDTH, src.type() );

	gettimeofday( &calc_time, NULL);

	cv::Rect ROI_top( 0, 16, WIDTH, 75 );
	top_raw = src( ROI_top );
	cv::Rect ROI_bottom( 0, 110, WIDTH, 40 );
	bottom_raw = src( ROI_bottom );

	cv::resize(top_raw, top_img, top_img.size() );		
	cv::resize(bottom_raw, bottom_img, bottom_img.size() );
	  
	gettimeofday( &end_time, NULL );
	get_time_diff( &time_diff, &start_time, &end_time );
    printf( "Time Elapsed: %ld.%06ld seconds\n", time_diff.tv_sec, time_diff.tv_usec );
    
    get_time_diff( &time_diff, &calc_time, &end_time );
    printf( "Remap time: %ld.%06ld seconds\n", time_diff.tv_sec, time_diff.tv_usec );


	// Display the images 
    cvNamedWindow( "Original", 1 );
    imshow( "Original", src );
    
    cvNamedWindow( "Top Mirror", 1 );
    imshow( "Top Mirror", top_raw );
    
    cvNamedWindow( "Top Stereo", 1 );
    imshow( "Top Stereo", top_img );
    
    cvNamedWindow( "Bottom Stereo", 1 );
    imshow( "Bottom Stereo", bottom_img );
    
    cvNamedWindow( "Bottom Mirror", 1 );    
    imshow( "Bottom Mirror", bottom_raw );

	
	cv::waitKey(0);
	
	// if 's' key is pressed, save the unwrapped image
//	if ( saved == 1179731 || saved == 1048691 )
//	{	
//	    int last_index = path.find_last_of("/");
//    	std::string img_name = path.substr(last_index+1, path.length()-1);
//		std::string prefix = "undistorted_";
//		img_name = prefix + img_name; 
//		std::string out_path = output_path + img_name;
//		std::cout<<"Saved undistorted image to: "<<out_path<<std::endl;
//		imwrite(out_path, undistorted_img);		
//	}
	return 0;
}
