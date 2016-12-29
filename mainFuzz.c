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

void printHexBytes(uint8_t * data, uint32_t byteCount);

void printHexBytes(uint8_t * data, uint32_t byteCount) {

	// Allocate output string
	char * outputBuf = calloc(2*byteCount + 1, sizeof(char));
	if(outputBuf == NULL) {
		fprintf(stderr, "Unable to allocate output buffer\n");
		abort();
	}

	// Convert to hex and copy into string
    for(uint32_t i = 0; i < byteCount; i++)
    {
		char temp[3];
        sprintf(temp, "%02x", data[i]);
        strncpy((outputBuf + (2*i)), temp, 2);
    }

    // Null-terminate string
    outputBuf[2 * byteCount] = 0;

    // Print string
    printf("%s\n", outputBuf);

    free(outputBuf);
}

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
		abort();
	}

	// Get the length of the file in bytes
	struct stat inputFileStat;
	stat(inputFileName, &inputFileStat);
	uint32_t inputFileSize = inputFileStat.st_size;

	printf("Input file has %d bytes\n", inputFileSize);

	// Read the input data from the file
	uint8_t * inputData;
	inputData = calloc(inputFileSize, sizeof(uint8_t));
	if(inputData == NULL) {
		fprintf(stderr, "Unable to allocate input data buffer\n");
		abort();
	}

	fread(inputData, sizeof(uint8_t), inputFileSize, inputFile);

	fclose(inputFile);

	// Hash the input data
	uint8_t outputData[hashBitLen];

	for(uint32_t bitsToIgnore = 0; bitsToIgnore < 8; bitsToIgnore++) {
		HashReturn returnVal;

		returnVal = Hash(hashBitLen, inputData, 8*inputFileSize - bitsToIgnore, outputData);
		
		if(returnVal != SUCCESS) {
			fprintf(stderr, "Error hashing data");
			abort();
		}

		// Print the hash of the input data
		printHexBytes(outputData, hashBitLen/8);
	}

	free(inputData);

	return 0;
}
