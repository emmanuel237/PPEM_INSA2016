//declaration file for sobel functions
/*definition of the data type for solbel_thread function*/
typedef struct
{
	unsigned char *y_ptr;//input to sobel
	unsigned char *ysobel_ptr;//output for sobel
	int rows;
	int cols;
}sobel_func_args;
//the first sobel function
void sobel(unsigned char*y, unsigned char* ysobel, int rows, int cols);// prototype for the sobel funtion
//sobel function without the border pixels calculation
void sobel_no_borders(unsigned char*y, unsigned char* ysobel, int rows, int cols);// prototype for the sobel funtion
//function computing the edge values for the sobel filter using a symetric padding
void sobel_borders(unsigned char*y, unsigned char* ysobel, int rows, int cols);
//function dividing the image in nb_blocks blocks uses the defined type
void image_divide(sobel_func_args* arg_tab, unsigned int nb_blocks, unsigned char *y, unsigned char *ySobel, unsigned int img_height, unsigned int img_width);

