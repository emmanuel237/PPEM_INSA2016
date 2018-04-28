#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#include<pthread.h>

#include <SDL.h>

#include "yuvDisplay.h"
#include "yuvRead.h"

//definition of the number of threads
#define NB_THREADS_APP 50//number of threads including the main thread
#define NB_THREADS NB_THREADS_APP -1 //number of threads used for the calculation
#define Question 2 // define used for conditional compilation
/*definition of the data type for solbel_thread function*/
typedef struct
{
	unsigned char *y_ptr;//input to sobel
	unsigned char *ysobel_ptr;//output for sobel
	int rows;
	int cols;
}sobel_func_args;


int stopThreads = 0;
void sobel_edges(unsigned char*y, unsigned char* ysobel, int rows, int cols);//function computing the edge values for the sobel filter using a symetric padding
void sobel(unsigned char*y, unsigned char* ysobel, int rows, int cols);// prototype for the sobel funtion
void *sobel2(void** arg);//function receiving arguments in an array
void *sobel3(void* arg);//fonction using the defined type for input arguments
void *sobel_tread(void* arg);//fonction using the defined type for input arguments and lunching a tread running the sobel filter
int main(int argc, char** argv) //part1
{
    // Declarations	
    unsigned char y[HEIGHT*WIDTH], ySobel[HEIGHT*WIDTH], u[HEIGHT*WIDTH/4], v[HEIGHT*WIDTH/4];
	int y1_size, y2_size, rows1, rows2, cols;//size of the blocks
	//unsigned char* y1_ptr, y2_ptr,ys1_ptr, ys2_ptr;// pointers on the two blocks
	//thread variables
	pthread_t sobel_thread_ptr[2];
    // Init display
    yuvDisplayInit(0,WIDTH,HEIGHT);

    // Open file
    initReadYUV(WIDTH, HEIGHT);
//first part of the Lab

	//paralization with two threads
#if Question == 1
	while (!stopThreads)
	{

		/*Read a frame*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		//spliting the video in two videos
		//size of the portions of the video
		rows1 = HEIGHT / 2;
		rows2 = HEIGHT - rows1;//+1
		cols = WIDTH;
		//cols2 = WIDTH - cols1;

		y1_size = (HEIGHT*WIDTH) / 2;
		y2_size = HEIGHT*WIDTH - y1_size;//+1
		//taking the pointers on the two zones
		unsigned char* y1_ptr = y;
		unsigned char* y2_ptr = y + sizeof(unsigned char)*(y1_size-1);
		unsigned char* ys1_ptr = ySobel;
		unsigned char* ys2_ptr = ySobel + sizeof(unsigned char)*(y1_size-1);
		//calling the two sobel fonctions for each block in the threads
		//execute the the filter on the second half first then on the first half
		//creating the arguments for the thread function
		void *arg1[] = { y1_ptr, ys1_ptr, &rows1, &cols};
		void *arg2[] = { y2_ptr, ys2_ptr, &rows2, &cols };

		//version in with the table slpited but no paralelizm
		/*sobel(y2_ptr, ys2_ptr, rows2, WIDTH);
		  sobel(y1_ptr, ys1_ptr, rows1, WIDTH);*/

		/*Process with two threads and arguments in arrays*/
		/*pthread_create(&sobel_thread_ptr[0], NULL, sobel2, arg2);
		pthread_create(&sobel_thread_ptr[1], NULL, sobel2, arg1);*/

		//taking the arguments
		sobel_func_args thread1_args = { .y_ptr = y1_ptr, .ysobel_ptr = ys1_ptr, .rows = rows1, .cols = cols };
		sobel_func_args thread2_args = { .y_ptr = y2_ptr, .ysobel_ptr = ys2_ptr, .rows = rows2, .cols = cols };


		/*version with two threads lunched in the main*/
		/*pthread_create(&sobel_thread_ptr[0], NULL, sobel3, (void*)&thread1_args);
		pthread_create(&sobel_thread_ptr[1], NULL, sobel3, (void*)&thread2_args);*/

		/*version with two threads but the with the second sobel filter lunched in another thread*/
		pthread_create(&sobel_thread_ptr[0], NULL, sobel3, (void*)&thread1_args);
		pthread_create(&sobel_thread_ptr[1], NULL, sobel_tread, (void*)&thread2_args);
		/* Display it*/
		yuvDisplay(0, ySobel, u, v);
     }
#endif
//Question 15 : running the application with N threads
#if Question == 2
	while (!stopThreads)
	{
		//variable for N threaded parallel version
		pthread_t sobel_thread_N_ptr[NB_THREADS];
		int rowsi[NB_THREADS];
		sobel_func_args threadi_args[NB_THREADS];
		//division of the image between threads with an overlapping of two rows
		rowsi[0] = HEIGHT / (NB_THREADS) + 2;
		for (int i = 1; i < NB_THREADS - 1; i++)
		{
			rowsi[i] = HEIGHT / (NB_THREADS) +  2;
		}
		rowsi[NB_THREADS - 1] = HEIGHT - (HEIGHT / (NB_THREADS)) * (NB_THREADS - 1)+2;//adding offset to the last block
		//calulating the adresses for the pointers to the image portions
		threadi_args[0]. y_ptr = y;
		threadi_args[0].ysobel_ptr = ySobel;
		threadi_args[0].rows = rowsi[0];
		threadi_args[0].cols = WIDTH;
		/*reading the video f*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		//intialisation of the other elements of the array
		for (int i = 1; i < NB_THREADS; i++)
		{
			threadi_args[i].y_ptr = threadi_args[i - 1].y_ptr + sizeof(unsigned char)*(rowsi[i-1] - 2)*WIDTH;
			threadi_args[i].ysobel_ptr = threadi_args[i - 1].ysobel_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*WIDTH;
			threadi_args[i].rows = rowsi[i];
			threadi_args[i].cols = WIDTH;
		}
	

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

	printf("Exit program\n");
	yuvFinalize(0);
	return 0;

	//non paralized version
	/*
    while(!stopThreads)
    {
        readYUV(WIDTH, HEIGHT,y,u,v);
		sobel(y, ySobel, HEIGHT, WIDTH);
        yuvDisplay(0, ySobel,u,v);
    }

    printf("Exit program\n");
    yuvFinalize(0);
	return 0; */

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



void sobel(unsigned char*y_input, unsigned char* ysobel, int rows, int cols)
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

}


//definition of the function not using the structure

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
		//symetric padding is applyed on border (they have the same value the edge)
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
}


//sobel function with new defined typedef
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

