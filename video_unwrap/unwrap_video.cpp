/*
*  A program which unwraps video taken from the VirtualME robot for vision
*  processing. 
*  
*  Currently simply saves each unwrapped, undistorted frame of video as  
*  separate top and bottom mirror images. The undistortion requires a
*  calibration file to be specified. 
*
*  Ben Selby, August 2013
*/

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <sys/time.h>
#include <string>
#include <fstream>

#define PI 3.141592654

// define the image parameters for cropping the mirror - should be a square
const int OFFSET_X = 96;
const int OFFSET_Y = 8;
const int WIDTH = 465;
const int HEIGHT = WIDTH;
const int RADIUS = WIDTH/2;

const std::string output_path = "output/";

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
	
	// height of the individual 'unwarped' sections
	int section_height = 10;		
	
	if ( argc < 2 ) 
    {
        printf( "Usage: %s <video_filename> <calibration_data.txt> <number of lines> [optional:<section height>-save ] \n", argv[0] );
        return -1;
    }
    
    if ( argc > 4 )
    {
	    // Check for the "save video flag"
    	if ( strcmp( "-s", argv[4] ) == 0 || strcmp( "-save", argv[4] ) == 0 )
    		save = true; 
    	else 
    	 	section_height = atoi( argv[4] );    	
    }
    
    if ( argc > 5 )
	{
		section_height = atoi( argv[5] );
    }
    

    
    // Read the pixel values of the lines from the text file and store them in
    // the array y_vals:
    int num_lines = atoi( argv[3] );
    std::ifstream input_data( argv[2] );
    std::string line;
    int y_vals[num_lines*2];
    
    printf("Reading calibration data file... ");
    if ( input_data.is_open() )
    {
    	int index = 0;
    	while ( input_data.good() )
    	{
    		getline( input_data, line );
    		y_vals[index] = atoi( line.c_str() );
//    		printf("line %d: %d\n", index, y_vals[index]);
    		index++;    		
    	}
    }
    else
    {
    	printf( "Unable to open the file \"%s\" - exiting.\n", argv[2] ); 
    	return -1;
    }
    
    input_data.close();    
	printf("done.\n");
    
	struct timeval start_time, end_time, time_diff, calc_time;
	gettimeofday(&start_time, NULL);

	std::string video_filename = argv[1];
	
	printf("Capturing video from '%s'...", argv[1] );
	cv::VideoCapture capture( video_filename );
	printf(" done.\n");	
	
	// Check if the capture object successfully initialized
	if ( !capture.isOpened() ) 
	{
		printf( "Failed to load video, exiting.\n" );
		return -1;
	}
	
	cv::Mat frame, cropped_img, unwrapped_img, top_img, bottom_img;
	cv::Mat map_x, map_y;
	cv::Rect ROI( OFFSET_X, OFFSET_Y, WIDTH, HEIGHT );
	
	// for now, read the first frame so we can create the map... 
	capture.read( frame );
	unwrapped_img.create( RADIUS, (int) 2*PI*RADIUS, frame.type() );

	// create the maps with same size as the cropped image	
	double UNWRAPPED_WIDTH = 2*PI*RADIUS;
	map_x.create( RADIUS, UNWRAPPED_WIDTH, CV_32FC1 );
	map_y.create( RADIUS, UNWRAPPED_WIDTH, CV_32FC1 );
	
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

	// get the top and bottom from the calibration data array:
	int top_upper = y_vals[0];
	int top_lower = y_vals[num_lines-1];
	int bottom_upper = y_vals[num_lines];
	int bottom_lower = y_vals[2*num_lines-1];
	
	int OUTPUT_HEIGHT = (num_lines-1)*section_height;
	
	// create the containers for the output stereo images
	top_img.create( OUTPUT_HEIGHT, UNWRAPPED_WIDTH, frame.type() );
	bottom_img.create( OUTPUT_HEIGHT, UNWRAPPED_WIDTH, frame.type() );
	cv::Mat section, resized_section;
	
	resized_section.create( section_height, UNWRAPPED_WIDTH, frame.type() );

	// video writer does not appear to be working, for now just output a series
	// of images

//	int fourcc = static_cast<int>(capture.get(CV_CAP_PROP_FOURCC));
//	double fps = 30;
//	cv::Size frame_size = cv::Size( RADIUS, (int)2*PI*RADIUS );
//	video_filename = "test.avi";
//	cv::VideoWriter writer( video_filename, fourcc, fps, frame_size );
//		
//	if ( !writer.isOpened() && save )
//	{
//		printf("Failed to initialize video writer, unable to save video!\n");
//	}
		
	int frame_num = 1; // the current frame index
	while(true)
	{	
		if ( !capture.read(frame) )
		{
			printf("Failed to read next frame, exiting.\n");
			break;
		}
		
		// select the region of interest in the frame
		cropped_img = frame( ROI );
				
		// Remap the image to unwrap it
	    cv::remap( cropped_img, unwrapped_img, 
    		 map_x,
    		 map_y,
    		 CV_INTER_LINEAR,
    		 cv::BORDER_CONSTANT,
    		 cv::Scalar(0,0,0)
		   );
		   
		// Perform the undistortion as specified by the input file:
		// Patch together resized image to produce images with uniform angular resolution
		for ( int i = 0; i<num_lines-1; i++ )
		{
			// for the top image...		
			section = unwrapped_img( cv::Rect( 0, 
												y_vals[i], 
												UNWRAPPED_WIDTH, 
												y_vals[i+1] - y_vals[i] ) );	
			
			cv::resize( section, resized_section, resized_section.size() );

			resized_section.copyTo( top_img( cv::Rect( 0, 
														i*section_height, 
														UNWRAPPED_WIDTH, 
														section_height ) ) );

			// and the bottom
			section = unwrapped_img( cv::Rect( 0, 
											y_vals[i+num_lines], 
											UNWRAPPED_WIDTH, 
											y_vals[i+num_lines+1] - y_vals[i+num_lines] ) );

			cv::resize( section, resized_section, resized_section.size() );

			resized_section.copyTo( bottom_img( cv::Rect( 0, 
														i*section_height, 
														UNWRAPPED_WIDTH, 
														section_height ) ) );
		}		   
		
		// display the images and wait
		imshow("unwrapped", unwrapped_img);
		imshow("bottom", bottom_img);
		imshow("top", top_img);
		
		// if we are saving video, write the unwrapped image		
		if (save)
		{
			char buff[50], buff2[50];
			sprintf( buff, "%stop_frame_%d.jpg", output_path.c_str(), frame_num );
			std::string out_name = buff;
			imwrite(out_name, top_img);
			
			sprintf( buff2, "%sbottom_frame_%d.jpg", output_path.c_str(), frame_num );
			out_name = buff2;
			imwrite(out_name, bottom_img);
//			writer << unwrapped_img;
		}
		frame_num++;
		
		char key = cv::waitKey(30);
		
		// if ESC is pressed, break
		if ( key == 27 ) 
		{			
			break;
		} 			
	}	
	
	
	cv::waitKey(0);
	return 0;
}
