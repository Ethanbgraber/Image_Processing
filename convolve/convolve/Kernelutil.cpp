// ================================================================
// Convutil.cpp - Read/Save PGM files.
//
// Copyright (C) 2014 by George Wolberg, 
//
// Written by: George Wolberg, 2014, Ethan Graber 2015
// ================================================================


//#include "stdafx.h"
#include "IP.h"


using namespace std;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IP_readKernel:
//
// Read image from file. The image is in PGM format.
//
kernelP
kP_readkernel(char *file)
{

	//imageP	 I;
	kernelP k;
	int	 w, h, maxGray;
	char  	 buf[1024];
	float	*out;

	// open binary file for reading
	ifstream inFile(file, ios::binary);

	// error checking
	if (!inFile) {
		cerr << "readImage: Can't open " << file << endl;
		exit(1);
	}

	/*
	// verify that the image is in PGM format. 
	inFile.getline(buf, 3);
	if (strncmp(buf, "P5", 2) && strncmp(buf, "P6", 2)){
		cerr << "The file " << file << " is not in PGM format" << endl;
		inFile.close();
		exit(1);
	}
	*/

	// read width, height
	inFile >> w >> h;

	// skip over linefeed and carriage return
	inFile.getline(buf, 2);

	// allocate image and read file
	k = kP_allockernel(w, h, sizeof(uchar));
	out = k->kernel;
	inFile.read((char *)out, w*h);

	return (kernelP)k;
}




/*

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IP_saveImage:
//
// Save image I to file in PGM format.
//
void
IP_saveImage(imageP I, char *file)
{

	uchar	*out;

	// open binary file for writing
	ofstream outFile(file, ios::binary);

	// error checking
	if (!outFile) {
		cerr << "IP_savePGM: Can't open " << file << endl;
		exit(1);
	}

	// save PGM header
	outFile << "P5" << endl;
	outFile << I->width << " " << I->height << endl;
	outFile << "255" << endl;

	// save image data
	out = I->image;
	outFile.write((char *)out, I->width*I->height);
	outFile.close();

}

*/

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// kP_allockernel:
//
// Allocate space for an image of width w and height h.
// Each pixel in the image has size s (in bytes).
// Return image structure pointer. 
//
kernelP
kP_allockernel(int w, int h, int s)
{
	float	*p;
	kernelP	 k;

	// allocate memory
	k = (kernelP)malloc(sizeof(kernelS));
	p = (float *)malloc(w * h * s);
	if (p == NULL) {
		cerr << "IP_allocImage: Insufficient memory\n";
		return ((kernelP)NULL);
	}

	// init structure
	k->width = w;
	k->height = h;
	k->kernel = p;

	return(k);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IP_freeImage:
//
// Free space of image I.
//
void
kP_freekernel(kernelP k)
{
	free((float *)k->kernel);
	free((float *)k);
}