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
#define nrLanes 25
#define nrRows 5
#define nrCols 5
uint64_t KeccakRoundConstants[nrRounds];
uint64_t KeccakRhoOffsets[nrRows][nrCols];

// Initialization
int LFSR86540(uint8_t *LFSR);
void KeccakInitializeRoundConstants();
void KeccakInitializeRhoOffsets();

// Absorbtion
void KeccakXorDataIntoState(SpongeMatrix state, const unsigned char *data, unsigned int dataLengthInBytes);

// Internal logic
#define index(x, y) ( ((x) % 5) + 5 * ((y) % 5) )

uint64_t ROL64(uint64_t a, unsigned int offset) {
    return ( ((uint64_t) a) << offset ) | ( ((uint64_t) a) >> (64 - offset) );
}

void theta(SpongeMatrix A);
void rho(SpongeMatrix A);
void pi(SpongeMatrix A);
void chi(SpongeMatrix A);
void iota(SpongeMatrix A, unsigned int indexRound);

void stateArrayToMatrix(unsigned char * state, SpongeMatrix stateMatrix) {
    unsigned int x, y;
    for(x = 0; x < nrRows; x++) {
        for(y = 0; y < nrCols; y++) {
            stateMatrix[x][y] = ((uint64_t *) state)[index(x, y)];
        }
    }
}

void stateMatrixToArray(SpongeMatrix state, unsigned char * stateArray) {
    unsigned int x, y;
    for(x = 0; x < nrRows; x++) {
        for(y = 0; y < nrCols; y++) {
            ((uint64_t *) stateArray)[index(x, y)] = state[x][y];
        }
    }
}

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

            if(LFSR86540(&LFSRstate)) {
                KeccakRoundConstants[i] ^= (uint64_t) 1 << bitPosition;
            }
        }
    }
}

void KeccakInitializeRhoOffsets()
{
    unsigned int x, y, newX, newY;

    KeccakRhoOffsets[0][0] = 0;

    x = 1;
    y = 0;

    unsigned int t;
    for(t = 0; t < 24; t++) {
        KeccakRhoOffsets[x][y] = ((t + 1) * (t + 2)/2) % 64;

        newX = (0 * x + 1 * y) % 5;
        newY = (2 * x + 3 * y) % 5;

        x = newX;
        y = newY;
    }
}

void KeccakInitialize(SpongeMatrix state)
{
    KeccakInitializeRoundConstants();
    KeccakInitializeRhoOffsets();

    memset(state, 0, sizeof(uint64_t) * nrRows * nrCols);
}

/*
 * Absorb and Permute
 */
void KeccakXorDataIntoState(SpongeMatrix state, const unsigned char *data, unsigned int dataLengthInBytes)
{
    unsigned char stateArray[KeccakPermutationSizeInBytes];
    unsigned int i;

    stateMatrixToArray(state, stateArray);

    for(i = 0; i < dataLengthInBytes; i++)
    {
        stateArray[i] ^= data[i];
    }

    stateArrayToMatrix(stateArray, state);
}

void KeccakPermutation(SpongeMatrix state)
{
    unsigned int round;
    for(round = 0; round < nrRounds; round++) {
        theta(state);
        rho(state);
        pi(state);
        chi(state);
        iota(state, round);
    }
}

void KeccakAbsorb(SpongeMatrix state, const unsigned char *data, unsigned int rate)
{
    KeccakXorDataIntoState(state, data, rate/8);
    KeccakPermutation(state);
}

/*
 * Keccak Round Steps
 */
void theta(SpongeMatrix A)
{
    unsigned int x, y;
    uint64_t C[5], D[5];

    for(x = 0; x < 5; x++) {
        C[x] = 0; 
        for(y = 0; y < 5; y++) {
            C[x] ^= A[x][y];
        }
    }
    for(x = 0; x < 5; x++) {
        D[x] = ROL64(C[(x + 1) % 5], 1) ^ C[(x + 4) % 5];
    }
    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            A[x][y] ^= D[x];
        }
    }
}

void rho(SpongeMatrix A)
{
    unsigned int x, y;

    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            A[x][y] = ROL64(A[x][y], KeccakRhoOffsets[x][y]);
        }
    }
}

void pi(SpongeMatrix A)
{
    unsigned int x, y;
    SpongeMatrix tempA;

    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            tempA[x][y] = A[x][y];
        }
    }
    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            uint8_t row = (0 * x + 1 * y) % 5;
            uint8_t col = (2 * x + 3 * y) % 5;
            A[row][col] = tempA[x][y];
        }
    }
}

void chi(SpongeMatrix A)
{
    unsigned int x, y;
    uint64_t C[5];

    for(y = 0; y < 5; y++) { 
        for(x = 0; x < 5; x++) {
            C[x] = A[x][y] ^ ((~A[(x + 1) % 5][y]) & A[(x + 2) % 5][y]);
        }
        for(x = 0; x < 5; x++) {
            A[x][y] = C[x];
        }
    }
}

void iota(SpongeMatrix A, unsigned int indexRound)
{
    A[0][0] ^= KeccakRoundConstants[indexRound];
}

/*
 * Squeezing
 */
void KeccakExtract(SpongeMatrix state, unsigned char *data, unsigned int rate)
{
    unsigned char stateArray[KeccakPermutationSizeInBytes];

    stateMatrixToArray(state, stateArray);

    memcpy(data, stateArray, rate/8);
}
