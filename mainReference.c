#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "KeccakNISTInterface.h"

#define RESET_COLOR   "\033[0m"
#define RED_COLOR     "\033[31m"
#define GREEN_COLOR   "\033[32m"

void KeccakN(uint32_t N, BitSequence * data, DataLength dataBitLen, char * outputBuf)
{
    BitSequence * output = calloc(sizeof(BitSequence), N/8);

    Hash(N, data, dataBitLen, output);

    char temp[10];
    uint32_t i;
    for(i = 0; i < (N/8); i++)
    {
        sprintf(temp, "%02x", output[i]);
        strncpy((outputBuf+(2*i)), temp, 2);
    }
    outputBuf[2*N/8] = 0;

    free(output);
}

uint32_t TestKeccakN(uint32_t N, char * inputData, uint32_t inputDataLen, char * expectedOutput)
{
    printf("Running Keccak%d on %d-bit message '%s'\n", N, (uint32_t) inputDataLen*8, inputData);

    char outputBuf[1000];

    KeccakN(N, (BitSequence *) inputData, inputDataLen*8, outputBuf);

    printf("Expected: %s\n", expectedOutput);

    if(strncmp(outputBuf, expectedOutput, N/8) == 0) {
        printf(GREEN_COLOR "Output:   %s\n" RESET_COLOR, outputBuf);
        printf("Test passed\n\n");
        return 0;
    } else {
        printf(RED_COLOR "Output:   %s\n" RESET_COLOR, outputBuf);
        printf("Test failed\n\n");
        return 1;
    }
}

int main()
{
    int testsFailed = 0;

    testsFailed += TestKeccakN(224, "", 0, "f71837502ba8e10837bdd8d365adb85591895602fc552b48b7390abd");

    testsFailed += TestKeccakN(256, "", 0, "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470");

    testsFailed += TestKeccakN(384, "", 0, "2c23146a63a29acf99e73b88f8c24eaa7dc60aa771780ccc006afbfa8fe2479b2dd2b21362337441ac12b515911957ff");

    testsFailed += TestKeccakN(512, "", 0, "0eab42de4c3ceb9235fc91acffe746b29c29a8c366b7c60e4e67c466f36a4304c00fa9caf9d87976ba469bcbe06713b435f091ef2769fb160cdab33d3670680e");

    testsFailed += TestKeccakN(256, "hello", 5, "1c8aff950685c2ed4bc3174f3472287b56d9517b9c948127319a09a7a36deac8");

    printf("%d Tests Failed\n", testsFailed);

    return 0;
}
