/*
 * Copyright 2016 Nathaniel Graff
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "KeccakNISTInterface.h"

#define hashBitLen 256

int main(int argc, char * argv[]) {
	
	// Check if we've been passed an input file
	if(argc < 2) {
		printf("Nothing to do\n");
		return 0;
	}

	// Open the input file
	FILE * inputFile;
	char inputFileName[100];
	strncpy(inputFileName, argv[1], 100);
	if((inputFile = fopen(inputFileName, "r")) == NULL) {
		printf("Error opening %s\n", inputFileName);
		return 1;
	}

	// Get the length of the file in bytes
	struct stat inputFileStat;
	stat(inputFileName, &inputFileStat);
	uint32_t inputFileSize = inputFileStat.st_size;

	printf("Input file has %d bytes\n", inputFileSize);

	// Read the input data from the file
	uint8_t * inputData;
	inputData = calloc(inputFileSize, sizeof(uint8_t));

	fread(inputData, sizeof(uint8_t), inputFileSize, inputFile);

	// Hash the input data
	uint8_t outputData[hashBitLen];

	HashReturn returnVal;
	returnVal = Hash(hashBitLen, inputData, 8*inputFileSize, outputData);
	if(returnVal != SUCCESS) {
		abort();
	}

	// Print the hash of the input data
	char outputBuf[2*hashBitLen/8 + 1];

	char temp[10];
    uint32_t i;
    for(i = 0; i < (hashBitLen/8); i++)
    {
        sprintf(temp, "%02x", outputData[i]);
        strncpy((outputBuf+(2*i)), temp, 2);
    }
    outputBuf[2*hashBitLen/8] = 0;

    printf("%s\n", outputBuf);

	return 0;
}
