/*
The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/

Implementation by the designers,
hereby denoted as "the implementer".

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#include "KeccakNISTInterface.h"
#include "KeccakSponge.h"

HashReturn Init(HashState * state, uint32_t hashbitlen)
{
    switch(hashbitlen) {
        case 0: // Default parameters, arbitrary length output
            InitSponge((SpongeState *) state, 1024, 576);
            break;
        case 224:
            InitSponge((SpongeState *) state, 1152, 448);
            break;
        case 256:
            InitSponge((SpongeState *) state, 1088, 512);
            break;
        case 384:
            InitSponge((SpongeState *) state, 832, 768);
            break;
        case 512:
            InitSponge((SpongeState *) state, 576, 1024);
            break;
        default:
            return BAD_HASHLEN;
    }
    state->fixedOutputLength = hashbitlen;
    return SUCCESS;
}

HashReturn Update(HashState * state, const BitSequence * data, DataLength databitlen)
{
    if ((databitlen % 8) == 0) {
        return Absorb((SpongeState *) state, data, databitlen);
    }
    else {
        HashReturn ret = Absorb((SpongeState*)state, data, databitlen - (databitlen % 8));
        if (ret == SUCCESS) {
            unsigned char lastByte; 
            // Align the last partial byte to the least significant bits
            lastByte = data[databitlen/8] >> (8 - (databitlen % 8));
            return Absorb((SpongeState*)state, &lastByte, databitlen % 8);
        }
        else {
            return ret;
        }
    }
}

HashReturn Final(HashState * state, BitSequence * hashval)
{
    return Squeeze(state, hashval, state->fixedOutputLength);
}

HashReturn Hash(uint32_t hashbitlen, const BitSequence *data, DataLength databitlen, BitSequence *hashval)
{
    HashState state;
    HashReturn result;

    if ((hashbitlen != 224) && (hashbitlen != 256) && (hashbitlen != 384) && (hashbitlen != 512)) {
        return BAD_HASHLEN; // Only the four fixed output lengths available through this API
    }

    result = Init(&state, hashbitlen);

    if (result != SUCCESS) {
        return result;
    }

    result = Update(&state, data, databitlen);
    
    if (result != SUCCESS) {
        return result;
    }

    result = Final(&state, hashval);
    
    return result;
}

