#include "IP.h"
//#include "ascii_file_float.cpp"
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
using namespace std;

// function prototype
void convolve(imageP, kernelP, imageP);
void cbuf(uchar**, imageP, int, int, int);
void fillPaddedBuffer(uchar*, int, uchar*, int, int);
//kernelP readKernel(string);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main:
//
// Main routine.
//
int main(int argc, char** argv)
{


	imageP    I1, I2;
	//string infile;
	kernelP kernel;

	// error checking: proper usage
	if (argc != 4) {
		cerr << "Usage: median infile sz avg_nbrs outfile\n";
		exit(1);
	}

	// read input image (I1) and reserve space for output (I2)
	I1 = IP_readImage(argv[1]);
	I2 = NEWIMAGE;

	// read xsize and ysize
	//kernel = kP_readkernel(argv[2]);
	//kernel = kP_readkernel(argv[2]);
	//for (int i = 0; i < 20; i++){
	//	cout << kernel->kernel[i] << endl;
	//}
	
	std::ifstream is(argv[2]);

	std::vector< std::vector<double> > matrix;
	load_matrix(&is, &matrix," \t");
	cout << "The matrix is:" << endl;
	
	int matrixtemp = 0;
	for (std::vector< std::vector<double> >::const_iterator it = matrix.begin(); it != matrix.end(); ++it)
	{
		for (std::vector<double>::const_iterator itit = it->begin(); itit != it->end(); ++itit)
			cout << *itit << '\t';
		//matrix[matrixtemp++] = it;
		cout << endl;
	}
	
	
	// call convolve and save result in file
	convolve(I1, kernel, I2);
	IP_saveImage(I2, argv[3]);

	// free up image structures/memory
	IP_freeImage(I1);
	IP_freeImage(I2);

	return 1;
}

void convolve(imageP I1, kernelP kernel, imageP I2) {
	float *kernel_buffer;

	// total number of pixels in image
	int total = I1->width * I1->height;


	// init I2 dimensions and buffer
	I2->width = I1->width;
	I2->height = I1->height;
	I2->image = (uchar *)malloc(total);
	if (I2->image == NULL) {
		cerr << "blur: insufficient memory\n";
		exit(1);
	}
	//
	kernel_buffer = kernel->kernel;
	//cout << kernel->kernel;
	//for (int i = 0; i < 20; ++i){
	//	float y = atof(kernel_buffer[i]); << endl;
	//}


	// init variables
	int i, j, currentRow, currentCol, sum;
	float temp;
	int	padsz = (kernel->width - 1) / 2;
	int medianIndex = (kernel->width*kernel->height - 1) / 2;

	vector<uchar> medianVector;

	// init buffer to be used by cbuf
	uchar** buffer = new uchar*[kernel->height];
	for (i = 0; i < kernel->height; ++i) {
		buffer[i] = new uchar[I1->width + kernel->width - 1];
	}

	//clear entire I2 with zeros
	for (i = 0; i < I2->width*I2->height; ++i){
		I2->image[i] = 0;
	}
	float tempa[9] = { -1, -1, -2, -1, 8, -1, -1, -1, -2 };

	// run the kernel over the circular buffer
	for (currentRow = 0; currentRow < I1->height; ++currentRow) {
		cbuf(buffer, I1, currentRow, padsz, kernel->height);
		// pass the kernel through the buffer
		for (currentCol = 0; currentCol < I1->width; ++currentCol) {
			//medianVector.clear();
			//sum = 0;
			for (i = 0; i < kernel->height; ++i) {
				int kernel_count = 0;
				temp = 0;
				for (j = -padsz; j <= padsz; ++j) {
					//medianVector.push_back(buffer[i][j + currentCol + padsz]);
					temp += tempa[kernel_count++] * buffer[i][j + currentCol + padsz];
				}
				I2->image[currentRow*I1->width + currentCol] = temp;
				
			}
		}
	}
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// cbuf:
//
// Circular buffer implementation.
//
void cbuf(uchar** buffer, imageP image, int currentRow, int padsz, int xsz) {
	int i;
	if (currentRow == 0) { // need to pad top if at beginning on the image
		// copy input array into buffer
		for (i = 0; i < padsz; ++i) {
			fillPaddedBuffer(buffer[i], sizeof(buffer[i]), image->image, image->width, padsz);
			fillPaddedBuffer(buffer[i + padsz + 1], sizeof(buffer[i]), image->image + i*image->width, image->width, padsz);
		}
		fillPaddedBuffer(buffer[padsz], sizeof(buffer[i]), image->image, image->width, padsz);
	}
	else if (currentRow >= image->height - padsz) {
		uchar* temp_buf = buffer[0];
		for (i = 0; i < xsz - 1; ++i) {
			buffer[i] = buffer[i + 1];
		}
		buffer[xsz] = temp_buf;
		fillPaddedBuffer(buffer[xsz], sizeof(buffer[xsz]), image->image + (image->height - 1)*image->width, image->width, padsz);
	}
	else {
		uchar* temp_buf = buffer[0];
		for (i = 0; i < xsz - 1; ++i) {
			buffer[i] = buffer[i + 1];
		}
		buffer[xsz] = temp_buf;
		fillPaddedBuffer(buffer[xsz], sizeof(buffer[xsz]), image->image + (currentRow + padsz)*image->width, image->width, padsz);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// fillPaddedBuffer:
//
// Fills a single buffer with padding for use by blur().
//
// insz is the width or height of the image.
//
void fillPaddedBuffer(uchar *buffer, int bsz, uchar *in, int insz, int pad)
{
	// padding (value extension) + copy array + padding (value extension)
	int i;
	for (i = 0; i < pad; ++i) {
		buffer[i] = in[0]; // pad front of buffer with first value of in[]
		buffer[bsz - i - 1] = in[insz - 1]; // pad end of buffer with last value of in[]
	}
	for (i = 0; i < insz; ++i) {
		buffer[pad + i] = in[i]; // insert in[] into buffer[] in between padding
	}
}
/*
kernelP readKernel(string filename) {
	int w, h;
	char c;
	ifstream infile;
	infile.open(filename, ios::in);

	if (infile)
	{
		while (infile.good())
			cout << (char)infile.get();
	}
	else
	{
		cout << "Unable to open file.";
	}
	infile.close();
	getchar();
	return 0;
}
*/