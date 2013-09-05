 /*
* This program rescales images taken from the virtualME nav camera and unwrapped
* to produce pseudo-stereo images for depth calculation. It requires the input
* of calibration information to be specified by the user. 
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

// specify input and output locations:
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
	if ( argc < 4 ) 
    {
        printf( "Usage: %s <image_filename> <cal_input_data.txt> <number_of_lines>\n", argv[0] );
        return -1;
    }    
    
    // Read the pixel values of the lines from the text file and store them in
    // the array y_vals:
    int num_lines = atoi( argv[3] );
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
	struct timeval start_time, end_time, time_diff, calc_time, resize_time;
	gettimeofday(&start_time, NULL);

	// Load the specified source image
	std::string path = argv[1];
	src = cv::imread( path, 1 );
	if ( !src.data ) 
	{
		printf( "Failed to load image, exiting.\n" );
		return -1;
	}

	// specify the dimensions of the output (piecewise-scaled) stereo images
	int HEIGHT = src.rows;
	int WIDTH = src.cols;

	top_img.create( HEIGHT, WIDTH, src.type() );
	bottom_img.create( HEIGHT, WIDTH, src.type() );

	gettimeofday( &calc_time, NULL);
	
	// get the top and bottom from the input data array:
	int top_upper = y_vals[0];
	int top_lower = y_vals[num_lines-1];
	int bottom_upper = y_vals[num_lines];
	int bottom_lower = y_vals[2*num_lines-1];
	
	// set the ROIs based on the calibration data
	cv::Rect ROI_top( 0, top_upper, WIDTH, top_lower-top_upper );
	top_raw = src( ROI_top );
	cv::Rect ROI_bottom( 0, bottom_upper, WIDTH, bottom_lower-bottom_upper );
	bottom_raw = src( ROI_bottom );

	gettimeofday( &resize_time, NULL );

	cv::Mat section, resized_section;
	int output_height = (int) HEIGHT/(num_lines-1);
	resized_section.create( output_height, WIDTH, src.type() );
	
	// patch together resized image to produce images with uniform angular resolution
	for ( int i = 0; i<num_lines-1; i++ )
	{
		// for the top image...		
		section = src( cv::Rect( 0, y_vals[i], WIDTH, y_vals[i+1] - y_vals[i] ) );	
		cv::resize( section, resized_section, resized_section.size() );
		resized_section.copyTo( top_img( cv::Rect( 0, i*output_height, WIDTH, output_height ) ) );
			
		// and the bottom
		section = src( cv::Rect( 0, y_vals[i+num_lines], WIDTH, y_vals[i+num_lines+1] - y_vals[i+num_lines] ) );
		cv::resize( section, resized_section, resized_section.size() );
		resized_section.copyTo( bottom_img( cv::Rect( 0, i*output_height, WIDTH, output_height ) ) );
	}
 
	gettimeofday( &end_time, NULL );
	get_time_diff( &time_diff, &start_time, &end_time );
    printf( "Time Elapsed: %ld.%06ld seconds\n", time_diff.tv_sec, time_diff.tv_usec );
    
    get_time_diff( &time_diff, &resize_time, &end_time );
    printf( "Resize time: %ld.%06ld seconds\n", time_diff.tv_sec, time_diff.tv_usec );

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

	char saved = cv::waitKey(0);
	
	// if 's' key is pressed, save the unwrapped image
	if ( saved == 83 || saved == 115 )
	{	
	    int last_index = path.find_last_of("/");
	    std::string top_name, bottom_name;
    	std::string img_name = path.substr(last_index+1, path.length()-1);
		top_name = "top_" + img_name;
		bottom_name = "bottom_" + img_name; 
		std::string out_path = output_path + top_name;
		imwrite(out_path, top_img);
		out_path = output_path + bottom_name;
		imwrite(out_path, bottom_img);	
		std::cout<<"Saved top image to: "<<out_path<<std::endl;
		std::cout<<"Saved top image to: "<<out_path<<std::endl;
	}
	return 0;
}
