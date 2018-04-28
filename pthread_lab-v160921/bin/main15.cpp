#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#include<pthread.h>

#include <SDL.h>

#include "yuvDisplay.h"
#include "yuvRead.h"

//definition of the number of threads
#define NB_TREADS 5 //number of threads including the main thread
/*definition of the data type for solbel_thread function*/
typedef struct
{
	unsigned char *y_ptr;
	unsigned char *ysobel_ptr;
	int rows;
	int cols;
}sobel_func_args;


int stopThreads = 0;
void sobel(unsigned char*y, unsigned char* ysobel, int rows, int cols);// prototype for the sobel funtion
void *sobel2(void** arg);//function receiving arguments in an array
void *sobel3(void* arg);//fonction using the defined type for input arguments
void *sobel_tread(void* arg);//fonction using the defined type for input arguments and lunching a tread running the sobel filter
int main(int argc, char** argv)
{
	// Declarations
	unsigned char y[HEIGHT*WIDTH], ySobel[HEIGHT*WIDTH], u[HEIGHT*WIDTH / 4], v[HEIGHT*WIDTH / 4];
	int y1_size, y2_size, rows1, rows2, cols;//size of the blocks
	//unsigned char* y1_ptr, y2_ptr,ys1_ptr, ys2_ptr;// pointers on the two blocks
	//thread variables
	pthread_t sobel_thread_ptr[2];
	// Init display
	yuvDisplayInit(0, WIDTH, HEIGHT);

	// Open file
	initReadYUV(WIDTH, HEIGHT);

	//paralization with two threads
	while (!stopThreads)
	{
		/*Read a frame*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		//spliting the video in two videos
		//size of the portions of the video
		rows1 = HEIGHT / 1;
		rows2 = HEIGHT - rows1;//+1
		cols = WIDTH;
		//cols2 = WIDTH - cols1;

		y1_size = (HEIGHT*WIDTH) / 2;
		y2_size = HEIGHT*WIDTH - y1_size;//+1
		//taking the pointers on the two zones
		unsigned char* y1_ptr = y;
		unsigned char* y2_ptr = y + sizeof(unsigned char)*(y1_size - 1);
		unsigned char* ys1_ptr = ySobel;
		unsigned char* ys2_ptr = ySobel + sizeof(unsigned char)*(y1_size - 1);
		//calling the two sobel fonctions for each block in the threads
		//execute the the filter on the second half first then on the first half
		//creating the arguments for the thread function
		void *arg1[] = { y1_ptr, ys1_ptr, &rows1, &cols };
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

	printf("Exit program\n");
	yuvFinalize(0);
	return 0;

	//non paralized version

	while (!stopThreads)
	{
		/*Read a frame*/
		readYUV(WIDTH, HEIGHT, y, u, v);
		/*apply sobel filter*/
		sobel(y, ySobel, HEIGHT, WIDTH);

		/* Display it*/
		yuvDisplay(0, ySobel, u, v);
	}

	printf("Exit program\n");
	yuvFinalize(0);

	return 0;
}

//fonction using the defined type for input arguments and lunching a tread running the sobel filter
void *sobel_tread(void* arg)
{
	pthread_t sobel_thread;
	//lunching the sobel function in a thread
	pthread_create(&sobel_thread, NULL, sobel3, (void*)arg);

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

