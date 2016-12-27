/*
 * Copyright 2016 Nathaniel Graff
 */

#include <stdint.h>
#include <string.h>

#include "KeccakNISTInterface.h"
#include "KeccakSponge.h"

HashReturn Init(HashState * state, uint32_t hashBitLen)
{
    HashReturn returnVal;

    switch(hashBitLen) {
        case 0: // Default parameters, arbitrary length output
            returnVal = InitSponge(state, 1024, 576);
            break;
        case 224:
            returnVal = InitSponge(state, 1152, 448);
            break;
        case 256:
            returnVal = InitSponge(state, 1088, 512);
            break;
        case 384:
            returnVal = InitSponge(state, 832, 768);
            break;
        case 512:
            returnVal = InitSponge(state, 576, 1024);
            break;
        default:
            returnVal = BAD_HASHLEN;
    }

    state->fixedOutputLength = hashBitLen;

    return returnVal;
}

HashReturn Update(HashState * state, const BitSequence * data, DataLength dataBitLen)
{
    if ((dataBitLen % 8) == 0) {
        return Absorb(state, data, dataBitLen);
    }
    else {
        HashReturn returnVal = Absorb(state, data, dataBitLen - (dataBitLen % 8));

        if (returnVal == SUCCESS) {
            // Align the last partial byte to the least significant bits
            uint8_t lastByte = data[dataBitLen / 8] >> (8 - (dataBitLen % 8));

            returnVal = Absorb(state, &lastByte, dataBitLen % 8);

            memset(&lastByte, 0, sizeof(lastByte)); // Clear memory of secret data

            return returnVal;
        }
        else {
            return returnVal;
        }
    }
}

HashReturn Final(HashState * state, BitSequence * hashVal)
{
    return Squeeze(state, hashVal, state->fixedOutputLength);
}

HashReturn Hash(uint32_t hashBitLen, const BitSequence * data, DataLength databitlen, BitSequence * hashVal)
{
    HashState state;
    HashReturn returnVal;

    if (hashBitLen == 0) {
        return BAD_HASHLEN; // Only the four fixed output lengths available through this API
    }

    returnVal = Init(&state, hashBitLen);

    if (returnVal != SUCCESS) {
        return returnVal;
    }

    returnVal = Update(&state, data, databitlen);
    
    if (returnVal != SUCCESS) {
        return returnVal;
    }

    returnVal = Final(&state, hashVal);

    EraseState(&state);
    
    return returnVal;
}

