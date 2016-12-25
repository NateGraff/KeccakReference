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

// Internal constants
#define nrRounds 24
uint64_t KeccakRoundConstants[nrRounds];
#define nrLanes 25
unsigned int KeccakRhoOffsets[nrLanes];

// Initialization
int LFSR86540(uint8_t *LFSR);
void KeccakInitializeRoundConstants();
void KeccakInitializeRhoOffsets();

// Absorbtion
void KeccakXorDataIntoState(unsigned char *state, const unsigned char *data, unsigned int dataLengthInBytes);

// Internal logic
#define index(x, y) ( ((x) % 5) + 5 * ((y) % 5) )

uint64_t ROL64(uint64_t a, unsigned int offset) {
    return ( ((uint64_t) a) << offset ) | ( ((uint64_t) a) >> (64 - offset) );
}

void theta(uint64_t *A);
void rho(uint64_t *A);
void pi(uint64_t *A);
void chi(uint64_t *A);
void iota(uint64_t *A, unsigned int indexRound);

/*
 * Keccak Initialization Functions
 */
int LFSR86540(uint8_t *LFSR)
{
    int result = ((*LFSR) & 0x01) != 0;
    if (((*LFSR) & 0x80) != 0) {
        // Primitive polynomial over GF(2): x^8+x^6+x^5+x^4+1
        (*LFSR) = ((*LFSR) << 1) ^ 0x71;
    }
    else {
        (*LFSR) <<= 1;
    }
    return result;
}

void KeccakInitializeRoundConstants()
{
    uint8_t LFSRstate = 0x01;
    unsigned int bitPosition;
    
    unsigned int i, j;
    for(i = 0; i < nrRounds; i++) {
        KeccakRoundConstants[i] = 0;

        for(j = 0; j < 7; j++) {
            bitPosition = (1 << j) - 1; // 2^j - 1

            if (LFSR86540(&LFSRstate)) {
                KeccakRoundConstants[i] ^= (uint64_t) 1 << bitPosition;
            }
        }
    }
}

void KeccakInitializeRhoOffsets()
{
    unsigned int x, y, newX, newY;

    KeccakRhoOffsets[index(0, 0)] = 0;

    x = 1;
    y = 0;

    unsigned int t;
    for(t = 0; t < 24; t++) {
        KeccakRhoOffsets[index(x, y)] = ((t + 1) * (t + 2)/2) % 64;

        newX = (0 * x + 1 * y) % 5;
        newY = (2 * x + 3 * y) % 5;

        x = newX;
        y = newY;
    }
}

void KeccakInitialize(unsigned char * state)
{
    KeccakInitializeRoundConstants();
    KeccakInitializeRhoOffsets();
    memset(state, 0, KeccakPermutationSizeInBytes);
}

/*
 * Absorb and Permute
 */
void KeccakXorDataIntoState(unsigned char *state, const unsigned char *data, unsigned int dataLengthInBytes)
{
    unsigned int i;

    for(i = 0; i < dataLengthInBytes; i++)
    {
        state[i] ^= data[i];
    }
}

void KeccakPermutation(unsigned char *state)
{
    unsigned int round;
    for(round = 0; round < nrRounds; round++) {
        theta((uint64_t*) state);
        rho((uint64_t*) state);
        pi((uint64_t*) state);
        chi((uint64_t*) state);
        iota((uint64_t*) state, round);
    }
}

void KeccakAbsorb(unsigned char *state, const unsigned char *data, unsigned int laneCount)
{
    KeccakXorDataIntoState(state, data, laneCount*8);
    KeccakPermutation(state);
}

/*
 * Keccak Round Steps
 */
void theta(uint64_t *A)
{
    unsigned int x, y;
    uint64_t C[5], D[5];

    for(x = 0; x < 5; x++) {
        C[x] = 0; 
        for(y = 0; y < 5; y++) {
            C[x] ^= A[index(x, y)];
        }
    }
    for(x = 0; x < 5; x++) {
        D[x] = ROL64(C[(x + 1) % 5], 1) ^ C[(x + 4) % 5];
    }
    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            A[index(x, y)] ^= D[x];
        }
    }
}

void rho(uint64_t *A)
{
    unsigned int x, y;

    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            A[index(x, y)] = ROL64(A[index(x, y)], KeccakRhoOffsets[index(x, y)]);
        }
    }
}

void pi(uint64_t *A)
{
    unsigned int x, y;
    uint64_t tempA[25];

    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            tempA[index(x, y)] = A[index(x, y)];
        }
    }
    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            A[index(0 * x + 1 * y, 2 * x + 3 * y)] = tempA[index(x, y)];
        }
    }
}

void chi(uint64_t *A)
{
    unsigned int x, y;
    uint64_t C[5];

    for(y = 0; y < 5; y++) { 
        for(x = 0; x < 5; x++) {
            C[x] = A[index(x, y)] ^ ((~A[index(x + 1, y)]) & A[index(x + 2, y)]);
        }
        for(x = 0; x < 5; x++) {
            A[index(x, y)] = C[x];
        }
    }
}

void iota(uint64_t *A, unsigned int indexRound)
{
    A[index(0, 0)] ^= KeccakRoundConstants[indexRound];
}

/*
 * Squeezing
 */

void KeccakExtract(const unsigned char *state, unsigned char *data, unsigned int laneCount)
{
    memcpy(data, state, laneCount*8);
}
