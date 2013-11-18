/*
*  A program which unwraps video taken from the VirtualME robot for vision
*  processing. 
*  
*  Currently simply saves each unwrapped, undistorted frame of video as  
*  separate top and bottom mirror images. The undistortion requires a
*  calibration file to be specified. 
*
*  Supports the inclusion of a .csv file containing the coordinates of the
*  centre of the mirror for stabilized unwrapped images.
*
*  Ben Selby, August 2013
*/

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <sys/time.h>
#include <string>
#include <fstream>
#include <iostream>

#define PI 3.141592654

// define the image parameters for cropping the mirror - should be a square
const int OFFSET_X = 96;
const int OFFSET_Y = 8;
//const int WIDTH = 465; // old width... ugh
const int WIDTH = 452;
const int HEIGHT = WIDTH;
const int RADIUS = WIDTH/2;
const double UNWRAPPED_WIDTH = 2*PI*RADIUS;

const std::string output_path = "output/";

//void update_maps( cv::Mat *x, cv::Mat *y, int frame_num, double[][] centre_coords);

int print_help()
{
    printf( "Usage: ./unwrap_video <video_filename> <calibration_data.txt> <number of lines> [optional: -height <section height> -save -centre <file.csv> ] \n");
    return -1;
}

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
	bool variable_centre = false;
	int centre_arg_num;	
	
	// height of the individual 'unwarped' sections
	int section_height = 10;		
	
	if ( argc < 2 ) 
    {
		return print_help();
    }
    
    if ( argc > 4 )
    {
    	for (int i = 4; i < argc; i++ )
    	{
    		if ( strcmp( "-s", argv[i] ) == 0 || strcmp( "-save", argv[i] ) == 0 )
    			save = true; 
    		else if ( strcmp( "-h", argv[i] ) == 0 || strcmp( "-height", argv[i] ) == 0 )
    		{
    			section_height = atoi( argv[i+1] );
	    		i++;
    		}
    		else if ( strcmp( "-c", argv[i] ) == 0 || strcmp( "-centre", argv[i] ) == 0 )
    		{
    			centre_arg_num = i+1;
    			variable_centre = true;
    			std::cout<<"Stabilization file found."<<std::endl;
    			i++;
    		}    		
			else 
			{
				std::cout<<"Invalid option \""<<argv[i]<<"\" specified, exiting."<<std::endl;
				return print_help();
			}
    	}    
    }

    // Read the pixel values of the lines from the text file and store them in
    // the array y_vals:
    int num_lines = atoi( argv[3] );
    std::ifstream input_data( argv[2] );
    std::string line;
    int y_vals[num_lines*2];
    int index; 
    float centre_coords[20000][2]; // for now, just hard-code the number of frames
    float x_coord, y_coord;
    
    
    printf("Reading calibration data file... ");
    if ( input_data.is_open() )
    {
    	index = 0;
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
	
	// if a centre-csv file was specified read it now:
	if ( variable_centre )
	{
		std::ifstream centre_data( argv[centre_arg_num] );
		if ( centre_data.is_open() )
		{
			index = 0;
			while( centre_data.good())
			{
				getline(centre_data, line);
				int found = line.find(",");
				x_coord = atof( line.substr( 0, found ).c_str() );
				y_coord = atof( line.substr( found+1, line.length() ).c_str() );
				centre_coords[index][0] = x_coord;
				centre_coords[index][1] = y_coord;
				index++;
//				printf("%f, %f\n", x_coord, y_coord);
			}
		}
		else 
		{
			printf( "Unable to open the centre data file - please specify a valid .csv.\n" ); 
			return -1;
		}
	}	
    
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

		// update the ROI with the variable centre for stabilization
		if (variable_centre)
		{
			float x_centre = centre_coords[frame_num][0];
			float y_centre = centre_coords[frame_num][1];
			std::cout<<x_centre - RADIUS<<" "<<y_centre - RADIUS<<" "<<WIDTH<<" "<<HEIGHT<<std::endl;
			cv::Rect tmp(x_centre - RADIUS, y_centre - RADIUS, WIDTH, HEIGHT);
			ROI = tmp;
			
		}
		std::cout<<ROI.x<<" "<<ROI.y<<" "<<ROI.width<<" "<<ROI.height<<std::endl;
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
//		imshow("unwrapped", unwrapped_img);
		imshow("bottom", bottom_img);
		imshow("top", top_img);
//		imshow("cropped", cropped_img);
//		imshow("raw", frame);
		
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
