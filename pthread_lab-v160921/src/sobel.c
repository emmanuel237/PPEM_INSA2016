#include"sobel.h"
#include"stdlib.h"

//definition of sobel functions
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

//this versions of the sobel filter does not  computations on the first and last rows and colunms
void sobel_no_borders(unsigned char*y_input, unsigned char* ysobel, int rows, int cols)
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
//function computing the border values for the sobel filter using a symetric padding
void sobel_borders(unsigned char*y_input, unsigned char* ysobel, int rows, int cols)
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
	gx = 2 * y_input[1] + y_input[cols + 1];
	gy = 2 * y_input[cols] + y_input[cols + 1];
	ysobel[0] = (abs(gx) + abs(gy)) / 8;
	//top corner right
	gx = -2 * y_input[cols - 2] + y_input[cols + cols - 2];
	gy = 2 * y_input[cols] + y_input[cols + 1];
	ysobel[cols - 1] = (abs(gx) + abs(gy)) / 8;
	//botton  left corner
	gx = 2 * y_input[1] + y_input[cols + 1];
	gy = 2 * y_input[cols*(rows - 2) + 1] + y_input[cols*(rows - 1) + 1];
	ysobel[cols*(rows - 1)] = (abs(gx) + abs(gy)) / 8;
	//botton  right corner
	gx = 2 * y_input[cols*(rows - 2) + cols - 2] + y_input[cols*(rows - 1) + cols - 2];
	gy = -2 * y_input[cols*(rows - 2) + 1] + y_input[cols*(rows - 1) + cols - 1];
	ysobel[cols*(rows - 1) + cols - 1] = (abs(gx) + abs(gy)) / 8;

}
//definition of the function in charge of the division of the input image in blocks
//each block overlaps the previous and the next of two rows
void image_divide(sobel_func_args* arg_tab, unsigned int nb_blocks, unsigned char *y, unsigned char *ySobel, unsigned int img_height, unsigned int img_width)
{
	unsigned int *rowsi; //array contening the number of rows for each block
	rowsi = malloc(sizeof(unsigned int)*nb_blocks);// dynamic allocation of memory for rowsi array
	//calculation of the number of rows for each block
	for (int i = 0; i < nb_blocks - 1; i++)
	{
		rowsi[i] = img_height / (nb_blocks)+2;
	}
	//the rest of the image is given to the last block
	rowsi[nb_blocks - 1] = img_height - (img_height / (nb_blocks)) * (nb_blocks - 1);
	//calulating the adresses for the pointers to the image portions
	arg_tab[0].y_ptr = y;
	arg_tab[0].ysobel_ptr = ySobel;
	arg_tab[0].rows = rowsi[0];
	arg_tab[0].cols = img_width;
	for (int i = 1; i < nb_blocks; i++)
	{
		arg_tab[i].y_ptr = arg_tab[i - 1].y_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*img_width;
		arg_tab[i].ysobel_ptr = arg_tab[i - 1].ysobel_ptr + sizeof(unsigned char)*(rowsi[i - 1] - 2)*img_width;
		arg_tab[i].rows = rowsi[i];
		arg_tab[i].cols = img_width;
	}
	free(rowsi);
}


