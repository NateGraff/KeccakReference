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

#include <stdint.h>
#include <string.h>

#include "KeccakSponge.h"
#include "KeccakF-1600-reference.h"

int InitSponge(spongeState *state, unsigned int rate, unsigned int capacity)
{
    if (rate+capacity != 1600) {
        return 1;
    }
    if ((rate <= 0) || (rate >= 1600) || ((rate % 64) != 0)) {
        return 1;
    }

    state->rate = rate;
    state->capacity = capacity;
    state->fixedOutputLength = 0;
    KeccakInitialize(state->state);
    
    memset(state->dataQueue, 0, KeccakMaximumRateInBytes);
    state->bitsInQueue = 0;
    state->squeezing = 0;
    state->bitsAvailableForSqueezing = 0;

    return 0;
}

void AbsorbQueue(spongeState *state)
{
    // state->bitsInQueue is assumed to be equal to state->rate

    KeccakAbsorb(state->state, state->dataQueue, state->rate/64);

    state->bitsInQueue = 0;
}

int Absorb(spongeState *state, const unsigned char *data, uint64_t dataBitLen)
{
    if ((state->bitsInQueue % 8) != 0) {
        return 1; // Only the last call may contain a partial byte
    }
 
    if (state->squeezing) {
        return 1; // Too late for additional input
    }

    uint64_t bitsAbsorbed = 0;

    while(bitsAbsorbed < dataBitLen) {

        if ((state->bitsInQueue == 0) && (dataBitLen >= state->rate) && (bitsAbsorbed <= (dataBitLen - state->rate))) {
            // If the data is at least a whole block and no data is waiting
            
            // How many whole blocks fit into the data
            uint64_t wholeBlocks = (dataBitLen - bitsAbsorbed) / state->rate;

            // Pointer to the current offset
            const unsigned char * curData = data + bitsAbsorbed/8;
            uint64_t block;

            // Absorb blocks
            for(block = 0; block < wholeBlocks; block++) {
                KeccakAbsorb(state->state, curData, state->rate/64);
                curData += state->rate/8;
            }

            bitsAbsorbed += wholeBlocks * state->rate;
        }
        else {
            // How much data is left to absorb
            uint64_t partialBlock = dataBitLen - bitsAbsorbed;

            // If the data left to absorb and the data in queue is greater than the rate,
            // process only as much new data as will fit after the data in queue.
            if (partialBlock + state->bitsInQueue > state->rate) {
                partialBlock = state->rate - state->bitsInQueue;
            }

            // Truncate to byte-align the new data length
            uint64_t partialByte = partialBlock % 8;
            partialBlock -= partialByte;

            // Append the new data to the queue
            memcpy(state->dataQueue + state->bitsInQueue/8, data + bitsAbsorbed/8, partialBlock/8);
            state->bitsInQueue += partialBlock;
            bitsAbsorbed += partialBlock;

            // Absorb the queue if it fills a whole block
            // A partial block will be left open for more data.
            // If it is the last data, the data will be padded prior to squeezing.
            if (state->bitsInQueue == state->rate) {
                AbsorbQueue(state);
            }

            // If a partial byte is left over
            if (partialByte > 0) {
                // Mask the remaining bits
                unsigned char mask = (1 << partialByte) - 1;

                // Add the masked bits to the queue
                state->dataQueue[state->bitsInQueue/8] = data[bitsAbsorbed/8] & mask;
                state->bitsInQueue += partialByte;
                bitsAbsorbed += partialByte;
            }
        }
    }
    return 0;
}

void PadAndSwitchToSqueezingPhase(spongeState *state)
{
    if (state->bitsInQueue + 1 == state->rate) {
        // If the queue is one bit short of a block
        state->dataQueue[state->bitsInQueue/8] |= 1 << (state->bitsInQueue % 8);
        AbsorbQueue(state);
        memset(state->dataQueue, 0, state->rate/8);
    }
    else {
        memset(state->dataQueue + (state->bitsInQueue + 7)/8, 0, state->rate/8 - (state->bitsInQueue + 7)/8);
        state->dataQueue[state->bitsInQueue/8] |= 1 << (state->bitsInQueue % 8);
    }

    state->dataQueue[(state->rate - 1)/8] |= 1 << ((state->rate - 1) % 8);
    AbsorbQueue(state);

    KeccakExtract(state->state, state->dataQueue, state->rate/64);
    state->bitsAvailableForSqueezing = state->rate;
    state->squeezing = 1;
}

int Squeeze(spongeState *state, unsigned char *output, uint64_t outputLength)
{
    if (!state->squeezing) {
        PadAndSwitchToSqueezingPhase(state);
    }

    if ((outputLength % 8) != 0) {
        return 1; // Only multiple of 8 bits are allowed, truncation can be done at user level
    }

    uint64_t bitsSqueezed = 0;
    uint64_t partialBlock = 0;

    while(bitsSqueezed < outputLength) {

        if (state->bitsAvailableForSqueezing == 0) {
            KeccakPermutation(state->state);

            KeccakExtract(state->state, state->dataQueue, state->rate/64);
            state->bitsAvailableForSqueezing = state->rate;
        }

        partialBlock = state->bitsAvailableForSqueezing;
        
        if(partialBlock > (outputLength - bitsSqueezed)) {
            partialBlock = outputLength - bitsSqueezed;
        }
        
        memcpy(output + bitsSqueezed/8, state->dataQueue + (state->rate - state->bitsAvailableForSqueezing)/8, partialBlock/8);
        
        state->bitsAvailableForSqueezing -= partialBlock;
        
        bitsSqueezed += partialBlock;

    }
    return 0;
}
