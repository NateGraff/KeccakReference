/*
 * Copyright 2016 Nathaniel Graff
 */

#include <stdint.h>
#include <string.h>

#include "KeccakSponge.h"
#include "KeccakF-1600-reference.h"

SpongeReturn InitSponge(SpongeState * state, uint32_t rate, uint32_t capacity)
{
    if (rate+capacity != 1600) {
        return BAD_RATE_CAPACITY;
    }
    if ((rate >= 1600) || ((rate % 64) != 0)) {
        return BAD_RATE_CAPACITY;
    }

    state->rate = rate;
    state->capacity = capacity;
    state->fixedOutputLength = 0;
    KeccakInitialize(state->state);
    
    memset(state->dataQueue, 0, KeccakMaximumRateInBytes);
    state->bitsInQueue = 0;
    state->mode = ABSORBING;
    state->bitsAvailableForSqueezing = 0;

    return SUCCESS;
}

SpongeReturn Absorb(SpongeState * state, const uint8_t * data, uint64_t dataBitLen)
{
    if ((state->bitsInQueue % 8) != 0) {
        return PARTIAL_BYTES_IN_MULTIPLE_ABSORBS; // Only the last call may contain a partial byte
    }
 
    if(state->mode == SQUEEZING) {
        return MODE_IS_SQUEEZING; // Too late for additional input
    }

    uint64_t bitsAbsorbed = 0;

    while(bitsAbsorbed < dataBitLen) {

        if ((state->bitsInQueue == 0) && (dataBitLen >= state->rate) && (bitsAbsorbed <= (dataBitLen - state->rate))) {
            // If the data is at least a whole block and no data is waiting
            
            // How many whole blocks fit into the data
            uint64_t wholeBlocks = (dataBitLen - bitsAbsorbed) / state->rate;

            // Pointer to the current offset
            const uint8_t * curData = data + bitsAbsorbed/8;
            uint64_t block;

            // Absorb blocks
            for(block = 0; block < wholeBlocks; block++) {
                KeccakAbsorb(state->state, curData, state->rate);
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
                KeccakAbsorb(state->state, state->dataQueue, state->rate);
                state->bitsInQueue = 0;
            }

            // If a partial byte is left over
            if (partialByte > 0) {
                // Mask the remaining bits
                uint8_t mask = (1 << partialByte) - 1;

                // Add the masked bits to the queue
                state->dataQueue[state->bitsInQueue/8] = data[bitsAbsorbed/8] & mask;
                state->bitsInQueue += partialByte;
                bitsAbsorbed += partialByte;
            }
        }
    }
    return SUCCESS;
}

void PadAndSwitchToSqueezingPhase(SpongeState * state)
{
    if (state->bitsInQueue + 1 == state->rate) { // The queue is one bit short of a block
        // The MSB is the first bit of the pad10*1.
        state->dataQueue[state->bitsInQueue/8] |= 1 << (state->bitsInQueue % 8);

        // Then absorb the queue as another whole block
        KeccakAbsorb(state->state, state->dataQueue, state->rate);
        state->bitsInQueue = 0;

        // Zero the queue to create a whole block of zeros
        memset(state->dataQueue, 0, state->rate/8);
    }
    else {
        // Set the first bit after the data to the first 1 of the pad10*1
        memset(state->dataQueue + (state->bitsInQueue + 7)/8, 0, state->rate/8 - (state->bitsInQueue + 7)/8);

        // Zero the queue after the data to create the 0*
        state->dataQueue[state->bitsInQueue/8] |= 1 << (state->bitsInQueue % 8);
    }

    // Set the final 1 of the pad10*1
    state->dataQueue[(state->rate - 1)/8] |= 1 << ((state->rate - 1) % 8);

    // Absorb the last block
    KeccakAbsorb(state->state, state->dataQueue, state->rate);
    state->bitsInQueue = 0;

    // Extract one block into the queue
    KeccakExtract(state->state, state->dataQueue, state->rate);
    state->bitsAvailableForSqueezing = state->rate;

    // Switch the sponge to squeezing mode
    state->mode = SQUEEZING;
}

SpongeReturn Squeeze(SpongeState * state, uint8_t * output, uint64_t outputLength)
{
    if (state->mode != SQUEEZING) {
        PadAndSwitchToSqueezingPhase(state);
    }

    if ((outputLength % 8) != 0) {
        // Only multiple of 8 bits are allowed, truncation can be done at user level
        return BAD_HASHLEN;
    }

    uint64_t bitsSqueezed = 0;
    uint64_t bitsToSqueeze = 0;

    while(bitsSqueezed < outputLength) {

        if (state->bitsAvailableForSqueezing == 0) {
            // Permute the state
            KeccakPermutation(state->state);

            // Extract another rate of bits
            KeccakExtract(state->state, state->dataQueue, state->rate);
            state->bitsAvailableForSqueezing = state->rate;
        }
        
        if(state->bitsAvailableForSqueezing > (outputLength - bitsSqueezed)) {
            // Extract as many bits as are left
            bitsToSqueeze = outputLength - bitsSqueezed;
        } else {
            // Extract as many bits as we can this round
            bitsToSqueeze = state->bitsAvailableForSqueezing;
        }
        
        memcpy(output + (bitsSqueezed / 8),
               state->dataQueue + ((state->rate - state->bitsAvailableForSqueezing) / 8),
               bitsToSqueeze / 8);
        
        state->bitsAvailableForSqueezing -= bitsToSqueeze;
        
        bitsSqueezed += bitsToSqueeze;

    }

    return SUCCESS;
}
