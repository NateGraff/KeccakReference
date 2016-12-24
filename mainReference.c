#include <stdio.h>

#include "KeccakNISTInterface.h"

void Keccak256(BitSequence * data, DataLength dataBitLen)
{
    const unsigned int outputBitLen = 256;
    BitSequence output[outputBitLen];

    Hash(outputBitLen, data, dataBitLen, output);

    printf("Running Keccak256 on %d-bit message '%s'\n", (unsigned int) dataBitLen, data);

    unsigned int i;
    for(i = 0; i < (outputBitLen/8); i++)
    {
        printf("%02x", output[i]);
    }
    printf("\n");
}

int main()
{
    Keccak256((BitSequence *) "", 0);
    printf("Expected value:\nc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470\n\n");

    Keccak256((BitSequence *) "hello", 5*8);
    printf("Expected value:\n1c8aff950685c2ed4bc3174f3472287b56d9517b9c948127319a09a7a36deac8\n\n");

    return 0;
}
