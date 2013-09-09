/*
*  A program which unwraps video taken from the VirtualME robot for vision
*  processing. 
*  
*  Ben Selby, August 2013
*/

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
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
	
	if ( argc < 2 ) 
    {
        printf( "Usage: %s <video_filename> [optional:-s (save video)] \n", argv[0] );
        return -1;
    }
    
    // Check for the "save video flag"
    if ( argc > 2 )
	{
	  	if ( strcmp( "-s", argv[2] ) == 0 )
    		save = true;
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
	
	cv::Mat frame, cropped_img, unwrapped_img;
	cv::Mat map_x, map_y;
	cv::Rect ROI( OFFSET_X, OFFSET_Y, WIDTH, HEIGHT );
	
	// for now, read the first frame so we can create the map... 
	capture.read( frame );
	unwrapped_img.create( RADIUS, (int) 2*PI*RADIUS, frame.type() );
	
	// create the maps with same size as the cropped image	
	map_x.create( RADIUS, (int) 2*PI*RADIUS, CV_32FC1 );
	map_y.create( RADIUS, (int) 2*PI*RADIUS, CV_32FC1 );
	
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

	int fourcc = static_cast<int>(capture.get(CV_CAP_PROP_FOURCC));
	double fps = 30;
	cv::Size frame_size = cv::Size( RADIUS, (int)2*PI*RADIUS );
	video_filename = "test.avi";
	
	// video writer does not appear to be working, for now just output a series
	// of images
//	cv::VideoWriter writer( video_filename, fourcc, fps, frame_size );
//		
//	if ( !writer.isOpened() && save )
//	{
//		printf("Failed to initialize video writer, unable to save video!\n");
//	}
		
	int frame_num = 1;
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
		   
		// display the images and wait
		imshow("cropped", cropped_img);
		imshow("unwrapped", unwrapped_img);
		
		// if we are saving video, write the unwrapped image
		
		if (save)
		{
			char buff[50];
			sprintf( buff, "%sunwrapped_frame_%d.jpg", output_path.c_str(), frame_num );
			printf("Name: %s\n", buff );
			std::string out_name = buff;
			imwrite(out_name, unwrapped_img);
//			writer << unwrapped_img;
		}
		frame_num++;
		
		char key = cv::waitKey(30);
		if ( key == 27 ) // if ESC is pressed, break
			break;		
	}


	cv::waitKey(0);
	return 0;
}
