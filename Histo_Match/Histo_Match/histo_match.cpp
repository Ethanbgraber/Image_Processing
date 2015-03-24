// ============================================================================
// qntz.cpp - Quantization program.
// Copyright (C) 2015 by Rosario Antunez, Steven Eisinger, and Ethan Graber
//
// Written by: Rosario Antunez, Steven Eisinger, and Ethan Graber, 2015
// ============================================================================

//#include "stdafx.h"
#include "IP.h"
#include <math.h>
using namespace std;

// function prototype
void qntz(imageP, int, imageP);
void histogramMatch(imageP I1, int n, imageP I2);
void histo_stretch(imageP I1, int t1, int t2, imageP I2);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main:
//
// Main routine to quantize image.
//
int main(int argc, char** argv)
{
	int	n, t1, t2;
	imageP	I1, I2;

	// error checking: proper usage
	if (argc != 4)
	{
		cerr << "Usage: qntz infile n outfile\n";
		exit(1);
	}

	// read input image (I1) and reserve space for output (I2)
	I1 = IP_readImage(argv[1]);
	I2 = NEWIMAGE;

	// read n
		n = atoi(argv[2]);
	//t1 = atoi(argv[2]);
	//t2 = atoi(argv[3]);

	// qntz image and save result in file
	//qntz(I1, n, I2);
	histogramMatch(I1, n, I2);
	//histo_stretch(I1, t1, t2, I2);
	IP_saveImage(I2, argv[3]);

	// free up image structures/memory
	IP_freeImage(I1);
	IP_freeImage(I2);

	return 1;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// qntz:
//
// Quantize I1 and save the output in I2.
// To devide 256 intensities into uniform intervals n = 2^k, where k in 0..8
// The following rules is used for quantazing:
//	
//	v_out = ceiling(v_in*n/256)*ceiling(128/n)
//	
//  No quantizing occurs when n = 256.
// PRECONDITION: n is a power of 2 in 1..256
void qntz(imageP I1, int n, imageP I2)
{
	int	 i, total;
	uchar	*in, *out, lut[256];

	// total number of pixels in image
	total = I1->width * I1->height;

	// init I2 dimensions and buffer
	I2->width = I1->width;
	I2->height = I1->height;
	I2->image = (uchar *)malloc(total);
	if (I2->image == NULL) {
		cerr << "qntz: Insufficient memory\n";
		exit(1);
	}

	// init lookup table
	for (i = 0; i < 256; i++)
	{
		//lut[i] = ((int) ceil(i*n /256)) * ((int) ceil(128/n));
		lut[i] = (((int)ceil(i*n / 256)) * 256 / n) + ((int)ceil(128 / n));
	}

	// visit all input pixels and apply lut to quantize
	in = I1->image;	// input  image buffer
	out = I2->image;	// output image buffer
	for (i = 0; i<total; i++) out[i] = lut[in[i]];
}

//HistogramMatch:
//
//
void histogramMatch(imageP I1, int n, imageP I2)
{
	int i, p, R, average, maxed;
	int left[MXGRAY], right[MXGRAY], maxinbin[MXGRAY], h2[MXGRAY];
	int total, Hsum, Havg, h1[MXGRAY];// h2[MXGRAY];
	unsigned char *in, *out;
	double scale;

	/* total number of pixels in image */
	total = (long)I1->height * I1->width;

	/* init I2 dimensions and buffer */
	I2->width = I1->width;
	I2->height = I1->height;
	I2->image = (unsigned char *)malloc(total);

	in = I1->image;				/* input image buffer */
	out = I2->image;			/* output image buffer */

	for (i = 0; i<MXGRAY; i++) h1[i] = 0; /* clear histogram */
	for (i = 0; i<total; i++) h1[in[i]]++;/* eval histogram */

	/* target histogram */
		//h2 = (int *)histo->image;
	/*The following three cases set up the shape of our histogram*/
	if (n == 0){
		average = (total / MXGRAY);
		for (i = 0; i<256; ++i) h2[i] = average;
	}	
	else if (n>0) 
		for (i = 0; i < MXGRAY; ++i) h2[i] = pow(i / MXGRAY,n);
	else if (n<0)
	for (i = 0; i < MXGRAY; ++i) h2[i] = pow(1 - i / MXGRAY, abs(n));
	for (i = 0; i < 256; i++){
		cout << h2[i] << ", ";
	}

	/*
	// normalize h2 to conform with dimensions of I1 
	for (i = Havg = 0; i<MXGRAY; i++) Havg += h2[i];
	scale = (double)total / Havg;
	if (scale != 1) for (i = 0; i<MXGRAY; i++) h2[i] *= scale;
	*/

	R = 0;
	Hsum = 0;
	/* evaluate remapping of all input gray levels;
	Each input gray value maps to an interval of valid output values.
	The endpoints of the intervals are left[] and right[] */
	for (i = 0; i<MXGRAY; i++) {
		left[i] = R;						/* left end of interval */
		Hsum += h1[i];						/* cumulative value for interval */
		while (Hsum>h2[R] && R<MXGRAY - 1) { /* compute width of interval */
			Hsum -= h2[R];					/* adjust Hsum as interval widens */
			R++;							/* update */
		}
		right[i] = R;						/* init right end of interval */
		maxinbin[R] = h2[R] - Hsum;
	}

	/* clear h1 and reuse it below */
	for (i = 0; i<MXGRAY; i++) h1[i] = 0;

	/* visit all input pixels */
	for (i = 0; i < total; i++) {
		p = left[in[i]];
		if ((h1[p] < h2[p]) && maxinbin[h1[p]]>0)			/* mapping satisfies h2 */
		
			out[i] = p;
		else out[i] = p = left[in[i]] = MIN(p + 1, right[in[i]]);
		h1[p]++;
	}
}

void histo_stretch(imageP I1, int t1, int t2, imageP I2){
	int i, p, R, f, g, average;
	int left[MXGRAY], right[MXGRAY], maxinbin[MXGRAY];
	int total, Hsum, Havg, h1[MXGRAY];// h2[MXGRAY];
	unsigned char *in, *out;
	double scale, h2[MXGRAY];

	/* total number of pixels in image */
	total = (long)I1->height * I1->width;

	/* init I2 dimensions and buffer */
	I2->width = I1->width;
	I2->height = I1->height;
	I2->image = (unsigned char *)malloc(total);

	in = I1->image;				/* input image buffer */
	out = I2->image;			/* output image buffer */

	for (i = 0; i<MXGRAY; i++) h1[i] = 0; /* clear histogram */
	for (i = 0; i<total; i++) h1[in[i]]++;/* eval histogram */

	/* target histogram */
	//h2 = (int *)histo->image;
	/*The following three cases set up the shape of our histogram*/
	//for (i = 0; i < MXGRAY; ++i) h2[i] = (((MXGRAY - 1)*(h1[i] - t1)) / (t2 - t1));

	for (f = 0; f < 256; ++f){
		g = (255 * (f - t1)) / (t2 - t1);
		if (0 < g < 255) h2[static_cast<int>(g)] = h1[f] * ((f - t1) / 256);
	}
	/*
	if (n == 0){
		average = (total / MXGRAY);
		for (i = 0; i<256; ++i) h2[i] = average;
	}
	else if (n>0)
	for (i = 0; i < MXGRAY; ++i) h2[i] = pow(i / MXGRAY, n);
	else if (n<0)
	for (i = 0; i < MXGRAY; ++i) h2[i] = pow(1 - i / MXGRAY, abs(n));
	for (i = 0; i < 256; i++){
		cout << h2[i] << ", ";
	}
	*/
	/*
	// normalize h2 to conform with dimensions of I1
	for (i = Havg = 0; i<MXGRAY; i++) Havg += h2[i];
	scale = (double)total / Havg;
	if (scale != 1) for (i = 0; i<MXGRAY; i++) h2[i] *= scale;
	*/

	R = 0;
	Hsum = 0;
	/* evaluate remapping of all input gray levels;
	Each input gray value maps to an interval of valid output values.
	The endpoints of the intervals are left[] and right[] */
	for (i = 0; i<MXGRAY; i++) {
		left[i] = R;						/* left end of interval */
		Hsum += h1[i];						/* cumulative value for interval */
		while (Hsum>h2[R] && R<MXGRAY - 1) { /* compute width of interval */
			Hsum -= h2[R];					/* adjust Hsum as interval widens */
			R++;							/* update */
		}
		right[i] = R;						/* init right end of interval */

	}

	/* clear h1 and reuse it below */
	for (i = 0; i<MXGRAY; i++) h1[i] = 0;

	/* visit all input pixels */
	for (i = 0; i < total; i++) {
		p = left[in[i]];
		if (h1[p] < h2[p])			/* mapping satisfies h2 */

			out[i] = p;
		else out[i] = p = left[in[i]] = MIN(p + 1, right[in[i]]);
		h1[p]++;
	}

}