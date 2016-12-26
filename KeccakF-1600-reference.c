/*
 * Copyright 2016 Nathaniel Graff
 */

#include <stdint.h>
#include <string.h>

#include "KeccakSponge.h"
#include "KeccakF-1600-reference.h"

uint64_t KeccakRoundConstants[nrRounds];
uint64_t KeccakRhoOffsets[nrRows][nrCols];

/*
 * Keccak Initialization Functions
 */
int32_t LFSR86540(uint8_t * LFSR)
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
    uint32_t bitPosition;
    
    uint32_t i, j;
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
    uint32_t x, y, newX, newY;

    KeccakRhoOffsets[0][0] = 0;

    x = 1;
    y = 0;

    uint32_t t;
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
void KeccakXorDataIntoState(SpongeMatrix state, const uint8_t * data, uint32_t dataLengthInBytes)
{
    uint8_t stateArray[KeccakPermutationSizeInBytes];

    stateMatrixToArray(state, stateArray);

    uint32_t i;
    for(i = 0; i < dataLengthInBytes; i++)
    {
        stateArray[i] ^= data[i];
    }

    stateArrayToMatrix(stateArray, state);
}

void KeccakPermutation(SpongeMatrix state)
{
    uint32_t round;
    for(round = 0; round < nrRounds; round++) {
        theta(state);
        rho(state);
        pi(state);
        chi(state);
        iota(state, round);
    }
}

void KeccakAbsorb(SpongeMatrix state, const uint8_t * data, uint32_t rate)
{
    KeccakXorDataIntoState(state, data, rate/8);
    KeccakPermutation(state);
}

/*
 * Keccak Round Steps
 */
uint64_t ROL64(uint64_t a, uint32_t offset) {
    return ( ((uint64_t) a) << offset ) | ( ((uint64_t) a) >> (64 - offset) );
}

void theta(SpongeMatrix A)
{
    uint32_t x, y;
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
    uint32_t x, y;

    for(x = 0; x < 5; x++) {
        for(y = 0; y < 5; y++) {
            A[x][y] = ROL64(A[x][y], KeccakRhoOffsets[x][y]);
        }
    }
}

void pi(SpongeMatrix A)
{
    uint32_t x, y;
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
    uint32_t x, y;
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

void iota(SpongeMatrix A, uint32_t indexRound)
{
    A[0][0] ^= KeccakRoundConstants[indexRound];
}

/*
 * Squeezing
 */
void KeccakExtract(SpongeMatrix state, uint8_t * data, uint32_t rate)
{
    uint8_t stateArray[KeccakPermutationSizeInBytes];

    stateMatrixToArray(state, stateArray);

    memcpy(data, stateArray, rate/8);
}

/*
 *  Matrix <-> Array conversion for easy Absorption and Extraction
 */
void stateArrayToMatrix(uint8_t * state, SpongeMatrix stateMatrix) {
    // Cast the 200-byte array into a 25-word array
    uint64_t * stateWordArray = (uint64_t * ) state;

    // Copy the array elements into the matrix
    uint32_t x, y;
    for(x = 0; x < nrRows; x++) {
        for(y = 0; y < nrCols; y++) {
            stateMatrix[x][y] = stateWordArray[x + (5 * y)];
        }
    }
}

void stateMatrixToArray(SpongeMatrix state, uint8_t * stateArray) {
    // Cast the 200-byte array into a 25-word array
    uint64_t * stateWordArray = (uint64_t * ) stateArray;

    // Copy the matrix elements into the array
    uint32_t x, y;
    for(x = 0; x < nrRows; x++) {
        for(y = 0; y < nrCols; y++) {
            stateWordArray[x + (5 * y)] = state[x][y];
        }
    }
}
