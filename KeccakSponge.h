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

#pragma once

#include <stdint.h>

#define KeccakPermutationSize 1600
#define KeccakPermutationSizeInBytes (KeccakPermutationSize/8)
#define KeccakMaximumRate 1536
#define KeccakMaximumRateInBytes (KeccakMaximumRate/8)

#if defined(__GNUC__)
#define ALIGN __attribute__ ((aligned(32)))
#elif defined(_MSC_VER)
#define ALIGN __declspec(align(32))
#else
#define ALIGN
#endif

typedef uint64_t SpongeMatrix[5][5];

typedef enum {
    SUCCESS,
    FAIL,
    BAD_HASHLEN,
    BAD_RATE_CAPACITY,
    MODE_IS_SQUEEZING,
    PARTIAL_BYTES_IN_MULTIPLE_ABSORBS,
} SpongeReturn;

typedef enum {
    ABSORBING,
    SQUEEZING,
} SpongeMode;

ALIGN typedef struct SpongeStateStruct {
    SpongeMatrix state;
    
    ALIGN uint8_t dataQueue[KeccakMaximumRateInBytes];
    uint32_t bitsInQueue;

    uint32_t rate;
    uint32_t capacity;
    
    uint32_t fixedOutputLength;

    SpongeMode mode;

    uint32_t bitsAvailableForSqueezing;
    
} SpongeState;

/**
  * Function to initialize the state of the Keccak[r, c] sponge function.
  * The sponge function is set to the absorbing phase.
  * @param  state       Pointer to the state of the sponge function to be initialized.
  * @param  rate        The value of the rate r.
  * @param  capacity    The value of the capacity c.
  * @pre    One must have r+c=1600 and the rate a multiple of 64 bits in this implementation.
  * @return SpongeReturn
  *         BAD_RATE_CAPACITY - The r and c values are invalid for KeccakF[1600]
  *         SUCCESS           - Sponge initialized
  */
SpongeReturn InitSponge(SpongeState * state, uint32_t rate, uint32_t capacity);

/**
  * Function to give input data for the sponge function to absorb.
  * @param  state       Pointer to the state of the sponge function initialized by InitSponge().
  * @param  data        Pointer to the input data. 
  *                     When @a databitLen is not a multiple of 8, the last bits of data must be
  *                     in the least significant bits of the last byte.
  * @param  databitLen  The number of input bits provided in the input data.
  * @pre    In the previous call to Absorb(), databitLen was a multiple of 8.
  * @pre    The sponge function must be in the absorbing phase,
  *         i.e., Squeeze() must not have been called before.
  * @return SpongeReturn
  *         PARTIAL_BYTES_IN_MULTIPLE_ABSORBS
  *                           - Two Absorb calls in a row have had partial bytes.
  *                           - Only the last call to Absorb may have a partial byte.
  *         MODE_IS_SQUEEZING - Squeezing has begun, no more data can be added.
  *         SUCCESS           - Sponge initialized
  */
SpongeReturn Absorb(SpongeState * state, const uint8_t * data, uint64_t dataBitLen);

/**
  * Function to squeeze output data from the sponge function.
  * If the sponge function was in the absorbing phase, this function 
  * switches it to the squeezing phase.
  * @param  state       Pointer to the state of the sponge function initialized by InitSponge().
  * @param  output      Pointer to the buffer where to store the output data.
  * @param  outputLength    The number of output bits desired.
  *                     It must be a multiple of 8.
  * @return SpongeReturn
  *         BAD_HASHLEN - The output length must be a multiple of whole bytes.
  *         SUCCESS     - Sponge initialized
  */
SpongeReturn Squeeze(SpongeState * state, uint8_t * output, uint64_t outputLength);

// Internal function for padding
void PadAndSwitchToSqueezingPhase(SpongeState * state);
