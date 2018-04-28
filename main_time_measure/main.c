#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#include<pthread.h>

#include <SDL.h>

#include "yuvDisplay.h"
#include "yuvRead.h"
#include "clock.h"

//definition of the number of threads
#define NB_THREADS_APP 2//number of threads including the main thread
#define Question 10// define used for conditional compilation
#define NB_THREADS NB_THREADS_APP -1 //number of threads used for the calculation
/*definition of the data type for solbel_thread function*/
typedef struct
{
	unsigned char *y_ptr;//input to sobel
	unsigned char *ysobel_ptr;//output for sobel
	int rows;
	int cols;
}sobel_func_args;
int stopThreads = 0;
pthread_barrier_t *read_barrier, *process_barrier; //barriers for synchronisation 
void sobel_edges(unsigned char*y, unsigned char* ysobel, int rows, int cols);//function computing the edge values for the sobel filter using a symetric padding
void sobel(unsigned char*y, unsigned char* ysobel, int rows, int cols);// prototype for the sobel funtion
void sobel1(unsigned char*y, unsigned char* ysobel, int rows, int cols);// prototype for the sobel funtion
void *sobel2(void** arg);//function receiving arguments in an array
void *sobel3(void* arg);//fonction using the defined type for input arguments
void *sobel_edges_tread_slave(void *arg);//function computing image edges without synchronization mecanism
void *sobel_edges_thread_func(void *arg);//function for the tread computing the value for the edge of the image
void *sobel_tread(void* arg);//fonction using the defined type for input arguments and lunching a tread running the sobel filter
void *master_thread_func(void** arg); //function executed by the master thread in charge of 
void *sobel_slv_thread(void* arg); //sobel function with the while(!stopThread) loop
void *sobel_slv_thread_barrier(void* arg);//function executed in the slave threads with barriers
int main(int argc, char** argv) //part1
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
//image processed by two no parallel calls and division of the source image by two
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
		sobel_edges(y,ySobel,HEIGHT,WIDTH); //process the edges
		//calling the non parallel functions
		sobel1(y1_ptr, ys1_ptr, rows1, WIDTH); //sobel1 function doesnt process edges
		sobel1(y2_ptr, ys2_ptr, rows2, WIDTH);
		yuvDisplay(0, ySobel, u, v);
     }

#endif
//parallel version with two threads and using the user defined type for argument
#if Question == 13 
	//division of the image in two blocks with an overlaping between blocks
	int rows1, rows2;
	pthread_t thread_ptr1, thread_ptr2;
	rows1 = HEIGHT/2 + 2;
	rows2 = HEIGHT - HEIGHT/2;
	//taking the pointers on the two zones
	unsigned char* y1_ptr = y;
	unsigned char* y2_ptr = y + sizeof(unsigned char)*(rows1-2)*WIDTH;
	unsigned char* ys1_ptr = ySobel;
	unsigned char* ys2_ptr = ySobel + sizeof(unsigned char)*(rows1-2)*WIDTH;
	//taking the arguments
	sobel_func_args thread1_args = { .y_ptr = y1_ptr, .ysobel_ptr = ys1_ptr, .rows = rows1, .cols = WIDTH };
	sobel_func_args thread2_args = { .y_ptr = y2_ptr, .ysobel_ptr = ys2_ptr, .rows = rows2, .cols = WIDTH };
	while (!stopThreads)
	{
		readYUV(WIDTH, HEIGHT, y, u, v);
		sobel_edges(y,ySobel,HEIGHT,WIDTH); //process the edges
		//calling the non parallel functions
		pthread_create(&thread_ptr1, NULL, sobel3, (void*)&thread1_args);
		pthread_create(&thread_ptr2, NULL, sobel3, (void*)&thread2_args);
		yuvDisplay(0, ySobel, u, v);
	}

#endif
	//paralization with two threads with the second thread being lunched in another function
#if Question == 14
	int rows1, rows2;
	pthread_t thread_ptr1, thread_ptr2;
	rows1 = HEIGHT/2 + 2;
	rows2 = HEIGHT - HEIGHT/2;
	//taking the pointers on the two zones
	unsigned char* y1_ptr = y;
	unsigned char* y2_ptr = y + sizeof(unsigned char)*(rows1-2)*WIDTH;
	unsigned char* ys1_ptr = ySobel;
	unsigned char* ys2_ptr = ySobel + sizeof(unsigned char)*(rows1 - 2)*WIDTH;
	//taking the arguments
	sobel_func_args thread1_args = { .y_ptr = y1_ptr, .ysobel_ptr = ys1_ptr, .rows = rows1, .cols = WIDTH };
	sobel_func_args thread2_args = { .y_ptr = y2_ptr, .ysobel_ptr = ys2_ptr, .rows = rows2, .cols = WIDTH };
	while (!stopThreads)
	{
		/*version with two threads but the with the second sobel filter lunched in another thread*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		sobel_edges(y,ySobel,HEIGHT,WIDTH); //process the edges
		pthread_create(&thread_ptr1, NULL, sobel3, (void*)&thread1_args);
		pthread_create(&thread_ptr2, NULL, sobel_tread, (void*)&thread2_args);
		/* Display it*/
		yuvDisplay(0, ySobel, u, v);
     }
#endif
//Question 15 : running the application with N threads 
#if Question == 15
	//variable for N threaded parallel version
	pthread_t sobel_thread_N_ptr[NB_THREADS];
	int rowsi[NB_THREADS];
	sobel_func_args threadi_args[NB_THREADS];
	//division of the image between threads with an overlapping of two rows
	for (int i = 0; i < NB_THREADS - 1; i++)
	{
		rowsi[i] = HEIGHT / (NB_THREADS)+2;
	}
	rowsi[NB_THREADS - 1] = HEIGHT - (HEIGHT / (NB_THREADS)) * (NB_THREADS - 1);
	//calulating the adresses for the pointers to the image portions
	threadi_args[0].y_ptr = y;
	threadi_args[0].ysobel_ptr = ySobel;
	threadi_args[0].rows = rowsi[0];
	threadi_args[0].cols = WIDTH;
	//intialisation of the other elements of the array
	for (int i = 1; i < NB_THREADS; i++)
	{
		threadi_args[i].y_ptr = threadi_args[i - 1].y_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*WIDTH;
		threadi_args[i].ysobel_ptr = threadi_args[i - 1].ysobel_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*WIDTH;
		threadi_args[i].rows = rowsi[i];
		threadi_args[i].cols = WIDTH;
	}
	while (!stopThreads)
	{
		/*reading the video file*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		//lunching the treads
		sobel_edges(y, ySobel, HEIGHT, WIDTH);
		for (int i = 0; i < NB_THREADS ; i++)
		{
			pthread_create(&sobel_thread_N_ptr[i], NULL, sobel3, (void*)&threadi_args[i]);
		}

		/* Display it*/
		yuvDisplay(0, ySobel, u, v);

	}
#endif
//Question 17 master and slave threats approach
//the main is the master threads and creates the slave threats
//unnecessary processing is done when the same frame are computed several times 
//we will try to measure this using printf
#if Question == 17
	//variable for N threaded parallel version
	pthread_t sobel_thread_N_ptr[NB_THREADS + 1];
	int rowsi[NB_THREADS];
	sobel_func_args threadi_args[NB_THREADS + 1];
	FILE *pfile; // a pointer on file to get the execution results in a file for futher analysis
	int exe_time;
	//division of the image between threads with an overlapping of two rows
	for (int i = 0; i < NB_THREADS - 1; i++)
	{
		rowsi[i] = HEIGHT / (NB_THREADS)+2;
	}
	rowsi[NB_THREADS - 1] = HEIGHT - (HEIGHT / (NB_THREADS)) * (NB_THREADS - 1);
	//calulating the adresses for the pointers to the image portions
	threadi_args[0].y_ptr = y;
	threadi_args[0].ysobel_ptr = ySobel;
	threadi_args[0].rows = rowsi[0];
	threadi_args[0].cols = WIDTH;
	//intialisation of the other elements of the array
	for (int i = 1; i < NB_THREADS; i++)
	{
		threadi_args[i].y_ptr = threadi_args[i - 1].y_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*WIDTH;
		threadi_args[i].ysobel_ptr = threadi_args[i - 1].ysobel_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*WIDTH;
		threadi_args[i].rows = rowsi[i];
		threadi_args[i].cols = WIDTH;
	}
	//parameters for the image edges processing thread
	threadi_args[NB_THREADS].y_ptr = y;
	threadi_args[NB_THREADS].ysobel_ptr = ySobel;
	threadi_args[NB_THREADS].rows = HEIGHT;
	threadi_args[NB_THREADS].cols = WIDTH;

	//creating the treads once for all
	//printf("lunching threads %i threads \n", NB_THREADS);
	for (int i = 0; i < NB_THREADS; i++)
	{
		pthread_create(&sobel_thread_N_ptr[i], NULL, sobel_slv_thread, (void*)&threadi_args[i]);
	}
	//lunching the edge processing thread
	pthread_create(&sobel_thread_N_ptr[NB_THREADS], NULL, sobel_edges_tread_slave, (void*)&threadi_args[NB_THREADS]);
	//opening the output file
	pfile = fopen("frame_read_delay.m", "w");
	fprintf(pfile, "frame_read_delay_measure = [\n");
	exe_time = 0;
	startTiming(0);
	while (!stopThreads)
	{
		/*reading the video file*/
		//we add some time mesurement to see how much unnecessery processing is done
		//we use the functions provided in the clock files
		readYUV(WIDTH, HEIGHT, y, u, v);
		exe_time = stopTiming(0);
		fprintf(pfile, "%i\n", exe_time);
		//printf("Execution time : %i\n", exe_time);
		startTiming(0);
		//apply the sobel filter on the edges of the image
		/* Display it*/
		yuvDisplay(0, ySobel, u, v);

	}
	fprintf(pfile, "];");
	fclose(pfile);
#endif

	//Question 18 master and slave threats approach
	//synchronism mecanism has  been added to synchronise the threads and avoid unnecessary processing
    // the synchronism mecanism chosen is the barriers 
#if Question == 18
	//variable for N threaded parallel version
	pthread_t sobel_thread_N_ptr[NB_THREADS + 1];
	int rowsi[NB_THREADS];
	sobel_func_args threadi_args[NB_THREADS + 1];
	//division of the image between threads with an overlapping of two rows
	for (int i = 0; i < NB_THREADS - 1; i++)
	{
		rowsi[i] = HEIGHT / (NB_THREADS)+2;
	}
	rowsi[NB_THREADS - 1] = HEIGHT - (HEIGHT / (NB_THREADS)) * (NB_THREADS - 1);
	//calulating the adresses for the pointers to the image portions
	threadi_args[0].y_ptr = y;
	threadi_args[0].ysobel_ptr = ySobel;
	threadi_args[0].rows = rowsi[0];
	threadi_args[0].cols = WIDTH;
	//intialisation of the other elements of the array
	for (int i = 1; i < NB_THREADS; i++)
	{
		threadi_args[i].y_ptr = threadi_args[i - 1].y_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*WIDTH;
		threadi_args[i].ysobel_ptr = threadi_args[i - 1].ysobel_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*WIDTH;
		threadi_args[i].rows = rowsi[i];
		threadi_args[i].cols = WIDTH;
	}
	//parameters for the thread computing the edges of the filtered image
	threadi_args[NB_THREADS].y_ptr = y;
	threadi_args[NB_THREADS].ysobel_ptr = ySobel;
	threadi_args[NB_THREADS].rows = HEIGHT;
	threadi_args[NB_THREADS].cols = WIDTH;

	//intialization of the barrier variables (we add 1 for the edge processing thread)
	pthread_barrier_init(read_barrier, NULL, NB_THREADS_APP + 1);// barrier to synchronize the reading of frames in the video file
	pthread_barrier_init(process_barrier, NULL, NB_THREADS_APP + 1);// barrier to synchronize the processing threads
	//creating the treads once for all
	//printf("lunching threads %i threads \n", NB_THREADS);
	for (int i = 0; i < NB_THREADS; i++)
	{
		pthread_create(&sobel_thread_N_ptr[i], NULL, sobel_slv_thread_barrier, (void*)&threadi_args[i]);
	}
	//lunching the edge processing thread
	pthread_create(&sobel_thread_N_ptr[NB_THREADS], NULL, sobel_edges_thread_func, (void*)&threadi_args[NB_THREADS]);
	while (!stopThreads)
	{
		/*reading the video file*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		pthread_barrier_wait(read_barrier);//synchonization to start processing when a new frame has been read
		/* Display it after been processed*/
		pthread_barrier_wait(process_barrier);//synchronization point to display the frame after it has been processed
		yuvDisplay(0, ySobel, u, v);

	}
#endif
	printf("Exit program\n");
	yuvFinalize(0);
	pthread_barrier_destroy(read_barrier);
	pthread_barrier_destroy(process_barrier);
	return 0;



}

//fonction using the defined type for input arguments and lunching a tread running the sobel filter
void *sobel_tread(void* arg)
{
	pthread_t sobel_thread;
	//lunching the sobel function in a thread
	pthread_create(&sobel_thread, NULL, sobel3, (void*)arg);

}
//function computing the edge values for the sobel filter using a symetric padding
void sobel_edges(unsigned char*y_input, unsigned char* ysobel, int rows, int cols)
{
	int gx, gy;
	for (int i = 1; i < rows - 1; i++) 
	{
		//symetric padding is appliyed on border (they have the same value the edge)
		//application of symetric padding on the first column
		gx = y_input[(i - 1)*cols + 1] - y_input[(i - 1)*cols] - 2 * y_input[i*cols] + 2 * y_input[i*cols + 1] - y_input[(i + 1)*cols] + y_input[(i + 1)*cols + 1];
		gy = y_input[(i + 1)*cols + 1] + 2 * y_input[(i + 1)*cols] + y_input[(i + 1)*cols] - y_input[(i - 1)*cols] - 2 * y_input[(i - 1)*cols] - y_input[(i - 1)*cols + 1];
		ysobel[(i)*cols] = (abs(gx) + abs(gy)) / 8;
		//application of symetric padding to the last column
		gx = y_input[(i - 1)*cols + (cols - 1)] - y_input[(i - 1)*cols + (cols - 1) - 1] - 2 * y_input[i*cols + (cols - 1) - 1] + 2 * y_input[i*cols + (cols - 1)] - y_input[(i + 1)*cols + (cols - 1) - 1] + y_input[(i + 1)*cols + (cols - 1)];
		gy = y_input[(i + 1)*cols + (cols - 1)] + 2 * y_input[(i + 1)*cols + (cols - 1)] + y_input[(i + 1)*cols + (cols - 1) - 1] - y_input[(i - 1)*cols + (cols - 1) - 1] - 2 * y_input[(i - 1)*cols + (cols - 1)] - y_input[(i - 1)*cols + (cols - 1)];
		ysobel[(i - 1)*cols + (cols - 1)] = ysobel[(i)*cols] = (abs(gx) + abs(gy)) / 8;//fills the last column with zeros

	}

	//symetric padding for the first and last rows
	for (int j = 1; j < cols - 1; j++)
	{   //the first row
		gx = y_input[j + 1] - y_input[j - 1] - 2 * y_input[j - 1] + 2 * y_input[j + 1] - y_input[cols + j - 1] + y_input[cols + j + 1];
		gy = y_input[cols + j + 1] + 2 * y_input[cols + j] + y_input[cols + j - 1] - y_input[cols + j - 1] - 2 * y_input[cols + j] - y_input[cols + j + 1];
		ysobel[j] = (abs(gx) + abs(gy)) / 8;
		//the last row
		gx = y_input[((rows - 1) - 1)*cols + j + 1] - y_input[((rows - 1) - 1)*cols + j - 1] - 2 * y_input[(rows - 1)*cols + j - 1] + 2 * y_input[(rows - 1)*cols + j + 1] - y_input[(rows - 1)*cols + j - 1] + y_input[(rows - 1)*cols + j + 1];
		gy = y_input[((rows - 1) + 1)*cols + j + 1] + 2 * y_input[(rows - 1)*cols + j] + y_input[(rows - 1)*cols + j - 1] - y_input[((rows - 1) - 1)*cols + j - 1] - 2 * y_input[((rows - 1) - 1)*cols + j] - y_input[((rows - 1) - 1)*cols + j + 1];
		ysobel[(rows - 1)*cols + j] = (abs(gx) + abs(gy)) / 8;
	}
	//compute the for edges pixels
	//top corner left
	gx = 2 * y_input[1] + y_input[cols+1];
	gy = 2 * y_input[cols] + y_input[cols + 1];
	ysobel[0] = (abs(gx) + abs(gy)) / 8;
    //top corner right
	gx = -2 * y_input[cols - 2] + y_input[cols + cols -2];
	gy = 2 * y_input[cols] + y_input[cols + 1];
	ysobel[cols-1] = (abs(gx) + abs(gy)) / 8;
	//botton  left corner
	gx = 2 * y_input[1] + y_input[cols + 1];
	gy = 2 * y_input[cols*(rows - 2) + 1] + y_input[cols*(rows - 1) + 1];
	ysobel[cols*(rows - 1)] = (abs(gx) + abs(gy)) / 8;
	//botton  right corner
	gx = 2 * y_input[cols*(rows - 2) + cols - 2] + y_input[cols*(rows - 1) + cols - 2];
	gy = -2 * y_input[cols*(rows - 2) + 1] + y_input[cols*(rows - 1) + cols- 1];
	ysobel[cols*(rows - 1)+ cols -1] = (abs(gx) + abs(gy)) / 8;

}
//function computing the soble edges without synchronization mecanism
void *sobel_edges_tread_slave(void *arg)
{
	unsigned char *y_input, *ysobel;
	int rows, cols;
	int gx, gy;
	y_input = ((sobel_func_args*)(arg))->y_ptr;//the filters input
	ysobel = ((sobel_func_args*)(arg))->ysobel_ptr;//the filters output
	rows = ((sobel_func_args*)(arg))->rows;
	cols = ((sobel_func_args*)(arg))->cols;
	while (!stopThreads)
	{
		//calling the function computing the value at the edges of the image
		sobel_edges(y_input, ysobel, rows, cols);
	}
	pthread_exit(NULL);
}

//Thread to compute the edges of the filtered image in a thread
void *sobel_edges_thread_func(void *arg)
{
	unsigned char *y_input, *ysobel;
	int rows, cols;
	int gx, gy;
	y_input = ((sobel_func_args*)(arg))->y_ptr;//the filters input
	ysobel = ((sobel_func_args*)(arg))->ysobel_ptr;//the filters output
	rows = ((sobel_func_args*)(arg))->rows;
	cols = ((sobel_func_args*)(arg))->cols;
	while (!stopThreads)
	{
	  //synchronization waiting for an image to be read
	  pthread_barrier_wait(read_barrier);
	  //calling the function computing the value at the edges of the image
	  sobel_edges(y_input, ysobel, rows, cols);
	  //synchronization point after calculations
	  pthread_barrier_wait(process_barrier);
	}
	pthread_exit(NULL);

}



void sobel(unsigned char*y_input, unsigned char* ysobel, int rows, int cols)
{
	int gx, gy;
	for (int i = 1; i < rows - 1; i++)
	{
		for (int j = 1; j < cols - 1; j++)
		{
			//calculating gx and gy here
			gx = y_input[(i - 1)*cols + j + 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[i*cols + j - 1] + 2 * y_input[i*cols + j + 1] - y_input[(i + 1)*cols + j - 1] + y_input[(i + 1)*cols + j + 1];
			gy = y_input[(i + 1)*cols + j + 1] + 2 * y_input[(i + 1)*cols + j] + y_input[(i + 1)*cols + j - 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[(i - 1)*cols + j] - y_input[(i - 1)*cols + j + 1];
			ysobel[i*cols + j] = (abs(gx) + abs(gy)) / 8;//compute the filters module

		}
		ysobel[(i - 1)*cols] = 0;//fills the first column with zeros
		ysobel[(i - 1)*cols + (cols - 1)] = 0;//fills the last column with zeros
	}
	//fills the first and last row of the filters output with zero
	for (int j = 1; j < cols - 1; j++)
	{
		ysobel[j] = 0;
		ysobel[(rows - 1)*cols + j] = 0;
	}
}


//these versions of the sobel filter do not make computation on the first and last rows and colunms
void sobel1(unsigned char*y_input, unsigned char* ysobel, int rows, int cols)
{
	 int gx, gy;
	 for (int i = 1; i < rows - 1; i++) // i current row
	 {
		 for (int j = 1; j < cols - 1; j++)  //j current col 
		 {
			 //calculating gx and gy here
			 gx = y_input[(i - 1)*cols + j + 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[i*cols + j - 1] + 2 * y_input[i*cols + j + 1] - y_input[(i + 1)*cols + j - 1] + y_input[(i + 1)*cols + j + 1];
			 gy = y_input[(i + 1)*cols + j + 1] + 2 * y_input[(i + 1)*cols + j] + y_input[(i + 1)*cols + j - 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[(i - 1)*cols + j] - y_input[(i - 1)*cols + j + 1];
			 ysobel[i*cols + j] = (abs(gx) + abs(gy)) / 8;//compute the filters module

		 }
		 

	 }
	 
}


//definition of the function passing arguments through array of pointer

void *sobel2(void** arg)
{
	unsigned char *y_input, *ysobel;
	int *rows_ptr, *cols_ptr;
	int rows, cols;
	int gx, gy;
	//reading the input arguments
	y_input = arg[0];//the filters input
	ysobel = arg[1];//the filters output
	rows_ptr = arg[2];
	cols_ptr = arg[3];
	rows = *rows_ptr;
	cols = *cols_ptr;

	for (int i = 1; i < rows - 1; i++)
	{
		for (int j = 1; j < cols - 1; j++)
		{
			//calculating gx and gy here
			gx = y_input[(i - 1)*cols + j + 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[i*cols + j - 1] + 2 * y_input[i*cols + j + 1] - y_input[(i + 1)*cols + j - 1] + y_input[(i + 1)*cols + j + 1];
			gy = y_input[(i + 1)*cols + j + 1] + 2 * y_input[(i + 1)*cols + j] + y_input[(i + 1)*cols + j - 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[(i - 1)*cols + j] - y_input[(i - 1)*cols + j + 1];
			ysobel[i*cols + j] = (abs(gx) + abs(gy)) / 8;//compute the filters module

		}
		
	}

}


//sobel function with new defined typedef
//this function does not compute the edge values for this are computed by overlaping the blocks
void *sobel3(void* arg)
{
	unsigned char *y_input, *ysobel;
	int rows, cols;
	int gx, gy;
	//reading the input arguments
	y_input = ((sobel_func_args*)(arg))->y_ptr;//the filters input
	ysobel = ((sobel_func_args*)(arg))->ysobel_ptr;//the filters output
	rows = ((sobel_func_args*)(arg))->rows;
	cols = ((sobel_func_args*)(arg))->cols;

	for (int i = 1; i < rows - 1; i++)
	{
		for (int j = 1; j < cols - 1; j++)
		{
			//calculating gx and gy here
			gx = y_input[(i - 1)*cols + j + 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[i*cols + j - 1] + 2 * y_input[i*cols + j + 1] - y_input[(i + 1)*cols + j - 1] + y_input[(i + 1)*cols + j + 1];
			gy = y_input[(i + 1)*cols + j + 1] + 2 * y_input[(i + 1)*cols + j] + y_input[(i + 1)*cols + j - 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[(i - 1)*cols + j] - y_input[(i - 1)*cols + j + 1];
			ysobel[i*cols + j] = (abs(gx) + abs(gy)) / 8;//compute the filters module

		}
		

	}
	
}

//sobel function with new defined typedef and the while(!stopThreads) loop
//this function does not compute the edge values for this are computed by overlaping the blocks
void *sobel_slv_thread(void* arg)
{
	unsigned char *y_input, *ysobel;
	int rows, cols;
	int gx, gy;
	//variable for execution time measure
	FILE *pfile; // a pointer on file to get the execution results in a file for futher analysis
	int exe_time;
	//reading the input arguments
	y_input = ((sobel_func_args*)(arg))->y_ptr;//the filters input
	ysobel = ((sobel_func_args*)(arg))->ysobel_ptr;//the filters output
	rows = ((sobel_func_args*)(arg))->rows;
	cols = ((sobel_func_args*)(arg))->cols;
	pfile = fopen("frame_process_delay.m", "w");
	fprintf(pfile, "frame_process_delay_measure = [\n");
	exe_time = 0;
	startTiming(10);
	while (!stopThreads)
	{
		exe_time = stopTiming(10);
		fprintf(pfile, "%i\n", exe_time);
		//printf("%i\n", exe_time);
		startTiming(10); 
		for (int i = 1; i < rows - 1; i++)
		{
			for (int j = 1; j < cols - 1; j++)
			{
				//calculating gx and gy here
				gx = y_input[(i - 1)*cols + j + 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[i*cols + j - 1] + 2 * y_input[i*cols + j + 1] - y_input[(i + 1)*cols + j - 1] + y_input[(i + 1)*cols + j + 1];
				gy = y_input[(i + 1)*cols + j + 1] + 2 * y_input[(i + 1)*cols + j] + y_input[(i + 1)*cols + j - 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[(i - 1)*cols + j] - y_input[(i - 1)*cols + j + 1];
				ysobel[i*cols + j] = (abs(gx) + abs(gy)) / 8;//compute the filters module

			}


		}
	}

	fprintf(pfile, "];");
	fclose(pfile);
	pthread_exit(NULL);
}
//sobel function for slave threads including the synchronisation points with barriers
void *sobel_slv_thread_barrier(void* arg)
{
	unsigned char *y_input, *ysobel;
	int rows, cols;
	int gx, gy;
	//reading the input arguments
	y_input = ((sobel_func_args*)(arg))->y_ptr;//the filters input
	ysobel = ((sobel_func_args*)(arg))->ysobel_ptr;//the filters output
	rows = ((sobel_func_args*)(arg))->rows;
	cols = ((sobel_func_args*)(arg))->cols;
	while (!stopThreads)
	{
		 //to do :
		//call the sobel function here instead of doing the loop
		pthread_barrier_wait(read_barrier);
		for (int i = 1; i < rows - 1; i++)
		{
			for (int j = 1; j < cols - 1; j++)
			{
				//calculating gx and gy here
				gx = y_input[(i - 1)*cols + j + 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[i*cols + j - 1] + 2 * y_input[i*cols + j + 1] - y_input[(i + 1)*cols + j - 1] + y_input[(i + 1)*cols + j + 1];
				gy = y_input[(i + 1)*cols + j + 1] + 2 * y_input[(i + 1)*cols + j] + y_input[(i + 1)*cols + j - 1] - y_input[(i - 1)*cols + j - 1] - 2 * y_input[(i - 1)*cols + j] - y_input[(i - 1)*cols + j + 1];
				ysobel[i*cols + j] = (abs(gx) + abs(gy)) / 8;//compute the filters module

			}

		}
		pthread_barrier_wait(process_barrier);
	}

	pthread_exit(NULL);
}

