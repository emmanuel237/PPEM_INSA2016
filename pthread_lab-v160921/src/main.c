#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#include<pthread.h>
#include<stdlib.h>
#include <SDL.h>
#include<string.h>
#include"sobel.h"
#include "yuvDisplay.h"
#include "yuvRead.h"
#include "clock.h"

//definition of the number of threads
#define NB_THREADS 2//number of threads including the main thread
#define Question 22// define used for conditional compilation
#define NB_THREADS_SOBEL NB_THREADS - 1 //number of threads executing the sobel function 
										   //one thread is dedicated to the execution of the sobel borders


int stopThreads = 0;
static pthread_barrier_t read_barrier, processing_barrier; //barriers for synchronisation 
void *sobel_borders_slv_thread(void *arg);//function for the thread computing the value for the borders of the image
void *sobel_borders_slv_thread_sync(void *arg);//function for the thread computing the value for the borders of the image with synchronization
void *sobel_borders_tread(void *arg); //thread processing the borders with the joint synchronization
void *sobel_thread(void* arg);//fonction using the defined type for input arguments and lunching a thread running the sobel filter
void *sobel_thread_slave(void* arg);//fonction using the defined type for input arguments and lunching a thread running the sobel filter
void *sobel_slv_thread(void* arg); //sobel function with the while(!stopThread) loop
void *sobel_slv_thread_sync(void* arg); //sobel function with the while(!stopThread) loop
int main(int argc, char** argv) 
{
    // Declarations	
    unsigned char y[HEIGHT*WIDTH], ySobel[HEIGHT*WIDTH], u[HEIGHT*WIDTH/4], v[HEIGHT*WIDTH/4];
    // Init display
    yuvDisplayInit(0,WIDTH,HEIGHT);
    // Open file
    initReadYUV(WIDTH, HEIGHT);
//first part of the Lab sobel filter as described in the pseudo algorithm
	//non paralized version
#if Question == 10
	while (!stopThreads)
	{
		readYUV(WIDTH, HEIGHT,y,u,v);
		sobel(y, ySobel, HEIGHT, WIDTH);
		yuvDisplay(0, ySobel,u,v);
		//yuvDisplay(0, y, u, v);
	}
#endif
//image processed by two non parallel calls and division of the source image by two
#if Question == 12 
	//division of the image in two blocks with an overlaping between blocks
	int rows1, rows2;
	rows1 = HEIGHT/2 + 2;
	rows2 = HEIGHT - HEIGHT/2;
	//taking the pointers on the two zones
	unsigned char* y1_ptr = y;
	unsigned char* y2_ptr = y + sizeof(unsigned char)*(rows1-2)*WIDTH;
	unsigned char* ys1_ptr = ySobel;
	unsigned char* ys2_ptr = ySobel + sizeof(unsigned char)*(rows1-2)*WIDTH;

	while (!stopThreads)
	{
		
		readYUV(WIDTH, HEIGHT, y, u, v);
		sobel_borders(y,ySobel,HEIGHT,WIDTH); //process the edges
		//calling the non parallel functions
		sobel_no_borders(y1_ptr, ys1_ptr, rows1, WIDTH); //sobel_no_border function doesnt process image borders
		sobel_no_borders(y2_ptr, ys2_ptr, rows2, WIDTH);
		yuvDisplay(0, ySobel, u, v);
     }

#endif
	//paralization with one sobel call excuted in a loop and another executed in a thread
#if Question == 14
	int rows1, rows2;
	pthread_t thread_ptr;
	rows1 = HEIGHT/2 + 2;
	rows2 = HEIGHT - HEIGHT/2;
	//taking the pointers on the two zones
	unsigned char* y1_ptr = y;
	unsigned char* y2_ptr = y + sizeof(unsigned char)*(rows1-2)*WIDTH;
	unsigned char* ys1_ptr = ySobel;
	unsigned char* ys2_ptr = ySobel + sizeof(unsigned char)*(rows1 - 2)*WIDTH;
	//taking the arguments
	sobel_func_args thread_args = { .y_ptr = y2_ptr, .ysobel_ptr = ys2_ptr, .rows = rows2, .cols = WIDTH };
	while (!stopThreads)
	{
		/*version with two threads but the with the second sobel filter lunched in another thread*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		pthread_create(&thread_ptr, NULL, sobel_thread, (void*)&thread_args);//executing the second half of the image in one thread
		sobel_borders(y,ySobel,HEIGHT,WIDTH); //process the edges
		sobel_no_borders(y1_ptr, ys1_ptr, rows1, WIDTH);//process a part of the image
		//synchronization point
		pthread_join(thread_ptr,NULL);
		/* Display it*/
		yuvDisplay(0, ySobel, u, v);
     }
#endif
//Question 15 : running the application with N threads 
#if Question == 15
	//variable for N threaded parallel version
	pthread_t sobel_thread_N_ptr[NB_THREADS_SOBEL];
	sobel_func_args sobel_args[NB_THREADS_SOBEL];
	//division of the image between threads with an overlapping of two rows
	image_divide(sobel_args, NB_THREADS_SOBEL, y, ySobel, HEIGHT, WIDTH);
	//infinite loop processing the video stream
	while (!stopThreads)
	{
		 
		/*reading the video file*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		//lunching the threads
		for (int i = 0; i < NB_THREADS_SOBEL; i++) 
		{
			pthread_create(&sobel_thread_N_ptr[i], NULL, sobel_thread, (void*)&sobel_args[i]);
		}
		//creating the synchronization point
		for (int i = 0; i < NB_THREADS_SOBEL; i++)
		{
			pthread_join(sobel_thread_N_ptr[i], NULL);
		}
		
		/* Display the image after its complete processing*/
		yuvDisplay(0, ySobel, u, v);
	}

#endif
//Question 17 master and slave threats approach
//the main is the master threads and creates the slave threats
//unnecessary processing is done when the same frame are computed several times 
//we will try to measure this using printf
#if Question == 17
	//variable for N threaded parallel version
	pthread_t sobel_thread_N_ptr[NB_THREADS_SOBEL];
	sobel_func_args sobel_args[NB_THREADS_SOBEL]; //arguments for the function computing the sobel filter
	sobel_func_args sobel_borders_args;//function for the sobel edge function

	//division of the image between threads with an overlapping of two rows
	image_divide(sobel_args, NB_THREADS_SOBEL - 1, y, ySobel, HEIGHT, WIDTH);
	//arguments for the soble_borders function
	sobel_borders_args.y_ptr = y;
	sobel_borders_args.ysobel_ptr = ySobel;
	sobel_borders_args.rows = HEIGHT;
	sobel_borders_args.cols = WIDTH;
	for (int i = 0; i < NB_THREADS_SOBEL -1; i++)
	{
		pthread_create(&sobel_thread_N_ptr[i], NULL, sobel_slv_thread, (void*)&sobel_args[i]);
	}
	//lunching the edge processing thread
	pthread_create(&sobel_thread_N_ptr[NB_THREADS_SOBEL - 1], NULL, sobel_borders_slv_thread, (void*)&sobel_borders_args);
	while (!stopThreads)
	{
		readYUV(WIDTH, HEIGHT, y, u, v);
		/* Display it*/
		yuvDisplay(0, ySobel, u, v);
	}

#endif

	//Question 18 master and slave threats approach
	//synchronism mecanism has  been added to synchronise the threads and avoid unnecessary processing
    // the synchronization mecanism chosen is the barriers 
#if Question == 18
	//variable for N threaded parallel version
	pthread_t sobel_thread_N_ptr[NB_THREADS_SOBEL];
	sobel_func_args sobel_args[NB_THREADS_SOBEL]; //arguments for the function computing the sobel filter
	//division of the image between threads with an overlapping of two rows
	image_divide(sobel_args, NB_THREADS_SOBEL, y, ySobel, HEIGHT, WIDTH);
	//intialization of the barrier variables (we add 1 for the edge processing thread)
	pthread_barrier_init(&read_barrier, NULL, NB_THREADS);// barrier to synchronize all the threads in the program
	pthread_barrier_init(&processing_barrier, NULL, NB_THREADS);// barrier to synchronize the processing threads
	//creating the sobel threads once for all
	for (int i = 0; i < NB_THREADS_SOBEL; i++)
	{
		pthread_create(&sobel_thread_N_ptr[i], NULL, sobel_slv_thread_sync, (void*)&sobel_args[i]);
	}
	//lunching the borders processing thread
	while (!stopThreads)
	{

		/*reading the video file*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		pthread_barrier_wait(&read_barrier);//synchonization to start processing when a new frame has been read
		pthread_barrier_wait(&processing_barrier);//synchronization point to display the frame after it has been processed
		/* Display it after been processed*/
		yuvDisplay(0, ySobel, u, v);

	}
	pthread_barrier_destroy(&read_barrier);
	pthread_barrier_destroy(&processing_barrier);
#endif
//implementation of the software pipelining 
#if Question == 22
	//variable for N threaded parallel version
	pthread_t sobel_thread_N_ptr[NB_THREADS_SOBEL];
	sobel_func_args sobel_args[NB_THREADS_SOBEL]; //arguments for the function computing the sobel filter
	unsigned char ybuffer[HEIGHT*WIDTH], ySobel_buffer[HEIGHT*WIDTH];
	//division of the image between threads with an overlapping of two rows
	image_divide(sobel_args, NB_THREADS_SOBEL, y, ySobel, HEIGHT, WIDTH);
	//intialization of the barrier variables (we add 1 for the edge processing thread)
	pthread_barrier_init(&read_barrier, NULL, NB_THREADS);// barrier to synchronize all the threads in the program
	pthread_barrier_init(&processing_barrier, NULL, NB_THREADS);// barrier to synchronize the processing threads
	//creating the sobel threads once for all
	for (int i = 0; i < NB_THREADS_SOBEL; i++)
	{
		pthread_create(&sobel_thread_N_ptr[i], NULL, sobel_slv_thread_sync, (void*)&sobel_args[i]);
	}
	//starting the pipelining
	readYUV(WIDTH, HEIGHT, ybuffer, u, v);//read the frame in the buffer
	memcpy(y, ybuffer, sizeof(unsigned char)*HEIGHT*WIDTH);//place the frame in the computing memory
	pthread_barrier_wait(&read_barrier);//crossing the sychronization point
	while (!stopThreads)
	{
		readYUV(WIDTH, HEIGHT, ybuffer, u, v);
		pthread_barrier_wait(&processing_barrier);//synchronization with the processing threads
		memcpy(y, ybuffer, sizeof(unsigned char)*HEIGHT*WIDTH);//buffering the new frame
		memcpy(ySobel_buffer, ySobel, sizeof(unsigned char)*HEIGHT*WIDTH);//taking the last results
		pthread_barrier_wait(&read_barrier); //new frame available for processing
		readYUV(WIDTH, HEIGHT, ybuffer, u, v);//reading a new frame
		yuvDisplay(0, ySobel, u, v);//display the processed frame
		pthread_barrier_wait(&processing_barrier);//synchronization with the processing threads
		memcpy(y, ybuffer, sizeof(unsigned char)*HEIGHT*WIDTH);//buffering the new frame
		memcpy(ySobel_buffer, ySobel, sizeof(unsigned char)*HEIGHT*WIDTH);//taking the last results
		pthread_barrier_wait(&read_barrier); //new frame available for processing
		yuvDisplay(0, ySobel, u, v);//display the processed frame

	}
	pthread_barrier_destroy(&read_barrier);
	pthread_barrier_destroy(&processing_barrier);
#endif


	printf("Exit program\n");
	yuvFinalize(0);

	return 0;
}

//question 14 : this thread is in charge of computing the second half of the image the first been computed in the main thread
void *sobel_thread(void* arg)
{
	sobel_func_args *sobel_args;
	sobel_args = (sobel_func_args*)arg;
	//lunching the sobel function in a thread
	sobel_no_borders(sobel_args->y_ptr, sobel_args->ysobel_ptr, sobel_args->rows, sobel_args->cols);//calling the sobel function
	pthread_exit(NULL);//destruction of the thread

}
//function for the slave threads without synchronization
//sobel function with new defined typedef and the while(!stopThreads) loop
//this function does not compute the edge values for this are computed by overlaping the blocks
void *sobel_slv_thread(void* arg)
{
	sobel_func_args *sobel_args;
	sobel_args = (sobel_func_args*)arg;
	while (!stopThreads)
	{
    	sobel_no_borders(sobel_args->y_ptr, sobel_args->ysobel_ptr, sobel_args->rows, sobel_args->cols);//calling the sobel function//exe_time = stopTiming(10);
	}

	pthread_exit(NULL);
}
//sobel thread with synchronization point
void *sobel_slv_thread_sync(void* arg)
{
	sobel_func_args *sobel_args;
	sobel_args = (sobel_func_args*)arg;
	while (!stopThreads)
	{
		pthread_barrier_wait(&read_barrier);
		sobel_no_borders(sobel_args->y_ptr, sobel_args->ysobel_ptr, sobel_args->rows, sobel_args->cols);//calling the sobel function//exe_time = stopTiming(10);
		pthread_barrier_wait(&processing_barrier);
	}

	pthread_exit(NULL);
}
//function for the thread computing the image borders with the join synchronization
void *sobel_borders_tread(void *arg)
{
	sobel_func_args *sobel_args;
	sobel_args = (sobel_func_args*)arg;
	sobel_borders(sobel_args->y_ptr, sobel_args->ysobel_ptr, sobel_args->rows, sobel_args->cols);
	pthread_exit(NULL);
}


//function computing the soble edges without synchronization mecanism
void *sobel_borders_slv_thread(void *arg)
{
	sobel_func_args *sobel_args;
	sobel_args = (sobel_func_args*)arg;
	while (!stopThreads)
	{

		sobel_borders(sobel_args->y_ptr, sobel_args->ysobel_ptr, sobel_args->rows, sobel_args->cols);//calling the sobel function//exe_time = stopTiming(10);

	}

	pthread_exit(NULL);
}
//function computing the soble edges with synchronization mecanism
void *sobel_borders_slv_thread_sync(void *arg)
{
	sobel_func_args *sobel_args;
	sobel_args = (sobel_func_args*)arg;
	while (!stopThreads)
	{
		pthread_barrier_wait(&read_barrier);
		sobel_borders(sobel_args->y_ptr, sobel_args->ysobel_ptr, sobel_args->rows, sobel_args->cols);//calling the sobel function//exe_time = stopTiming(10);
		pthread_barrier_wait(&processing_barrier);
	}

	pthread_exit(NULL);
}







