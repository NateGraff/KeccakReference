#include <stdio.h>
#include <stdlib.h>

#include "KeccakNISTInterface.h"

void KeccakN(unsigned int N, BitSequence * data, DataLength dataBitLen)
{
    BitSequence * output = calloc(sizeof(BitSequence), N/8);

    Hash(N, data, dataBitLen, output);

    printf("Running Keccak%d on %d-bit message '%s'\n", N, (unsigned int) dataBitLen, data);

    unsigned int i;
    for(i = 0; i < (N/8); i++)
    {
        printf("%02x", output[i]);
    }
    printf("\n");

    free(output);
}

int main()
{
    KeccakN(224, (BitSequence *) "", 0);
    printf("Expected value:\nf71837502ba8e10837bdd8d365adb85591895602fc552b48b7390abd\n\n");

    KeccakN(256, (BitSequence *) "", 0);
    printf("Expected value:\nc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470\n\n");

    KeccakN(384, (BitSequence *) "", 0);
    printf("Expected value:\n2c23146a63a29acf99e73b88f8c24eaa7dc60aa771780ccc006afbfa8fe2479b2dd2b21362337441ac12b515911957ff\n\n");

    KeccakN(512, (BitSequence *) "", 0);
    printf("Expected value:\n0eab42de4c3ceb9235fc91acffe746b29c29a8c366b7c60e4e67c466f36a4304c00fa9caf9d87976ba469bcbe06713b435f091ef2769fb160cdab33d3670680e\n\n");

    KeccakN(256, (BitSequence *) "hello", 5*8);
    printf("Expected value:\n1c8aff950685c2ed4bc3174f3472287b56d9517b9c948127319a09a7a36deac8\n\n");

    return 0;
}
