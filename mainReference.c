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
        printf("%02X", output[i]);
    }
    printf("\n\n");
}

int main()
{
    Keccak256((BitSequence *) "", 0);
    Keccak256((BitSequence *) "hello", 5*8);

    return 0;
}
