#include "IP.h"
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
using namespace std;

// function prototype
void convolve(imageP, std::vector< std::vector<double> >, imageP);
void cbuf(uchar**, imageP, int, int, int);
void fillPaddedBuffer(uchar*, int, uchar*, int, int);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main:
//
// Main routine.
//
int main(int argc, char** argv)
{


	imageP    I1, I2;

	// error checking: proper usage
	if (argc != 4) {
		cerr << "Usage: convolve infile matrix_file outfile\n";
		exit(1);
	}

	// read input image (I1) and reserve space for output (I2)
	I1 = IP_readImage(argv[1]);
	I2 = NEWIMAGE;

	//input Matrix file and save it to a vector
	std::ifstream is(argv[2]);
	std::vector< std::vector<double> > matrix;
	load_matrix(&is, &matrix," \t");
	// print out the matrix to double check it inputs correctly
	cout << "The matrix is:" << endl;
	for (std::vector< std::vector<double> >::const_iterator it = matrix.begin(); it != matrix.end(); ++it)
	{
		for (std::vector<double>::const_iterator itit = it->begin(); itit != it->end(); ++itit)
			cout << *itit << '\t';

		cout << endl;
	}

	// call convolve and save result in file
	convolve(I1, matrix, I2);
	IP_saveImage(I2, argv[3]);

	// free up image structures/memory
	IP_freeImage(I1);
	IP_freeImage(I2);

	return 1;
}

void convolve(imageP I1, std::vector< std::vector<double> > matrix, imageP I2) {

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
	//get width and height from first row, col 1 and 2
	int h = matrix.at(0).at(0);
	int w = matrix.at(0).at(1);
	
	// init variables
	int i, j, currentRow, currentCol;
	double temp;
	int	padsz = (w - 1) / 2;

	// init buffer to be used by cbuf
	uchar** buffer = new uchar*[h];
	for (i = 0; i < h; ++i) {
		buffer[i] = new uchar[I1->width + w - 1];
	}

	// run the kernel over the circular buffer
	for (currentRow = 0; currentRow < I1->height; ++currentRow) {
		cbuf(buffer, I1, currentRow, padsz, h);
		// pass the kernel through the buffer
		for (currentCol = 0; currentCol < I1->width; ++currentCol) {
			//temp always gets cleared when the matrix moves positions
			temp = 0;
			for (i = 0; i < h; ++i) {
				for (j = -padsz; j <= padsz; ++j) {
					//temp is updated with each multiplication in the matrix that corresponds to a pixel in the buffer 
					temp += matrix.at(i+1).at(j+padsz) * buffer[i][j + currentCol + padsz];
				}
			}
			
			//can only be a number between 0 and 255
			if (temp < 0){
				//add to I2 in current position
				I2->image[currentRow*I1->width + currentCol] = 0;
			}
			else if (temp > 255) {
				//add to I2 in current position
				I2->image[currentRow*I1->width + currentCol] = 255;
			}
			//add to I2 in current position
			else I2->image[currentRow*I1->width + currentCol] = temp;
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
