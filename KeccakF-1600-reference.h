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

#include "KeccakSponge.h"

#define nrRounds 24
#define nrLanes  25
#define nrRows    5
#define nrCols    5

/**
  * Initialize the sponge matrix and constants.
  * @param  state       Pointer to the sponge matrix.
  */
void KeccakInitialize(SpongeMatrix state);

/**
  * Absorb the input data into the sponge matrix.
  * @param  state       Pointer to the sponge matrix.
  * @param  data        Pointer to the input data.
  * @param  rate        Rate of the KeccakF algorithm
  */
void KeccakAbsorb(SpongeMatrix state, const uint8_t * data, uint32_t rate);

/**
  * Run a permutation of KeccakF.
  * @param  state       Pointer to the sponge matrix.
  */
void KeccakPermutation(SpongeMatrix state);

/**
  * Absorb the input data into the sponge matrix.
  * @param  state       Pointer to the sponge matrix.
  * @param  data        Pointer to the output data.
  * @param  rate        Rate of the KeccakF algorithm
  */
void KeccakExtract(SpongeMatrix state, uint8_t * data, uint32_t rate);

/*
 * Internal round functions
 */

// Initialization
int32_t LFSR86540(uint8_t * LFSR);
void KeccakInitializeRoundConstants();
void KeccakInitializeRhoOffsets();

// Matrix <-> Array Conversion
void stateArrayToMatrix(uint8_t * state, SpongeMatrix stateMatrix);
void stateMatrixToArray(SpongeMatrix state, uint8_t * stateArray);

// Absorbtion
void KeccakXorDataIntoState(SpongeMatrix state, const uint8_t * data, uint32_t dataLengthInBytes);

// Permutation
uint64_t ROL64(uint64_t a, uint32_t offset);

void theta(SpongeMatrix A);
void rho(SpongeMatrix A);
void pi(SpongeMatrix A);
void chi(SpongeMatrix A);
void iota(SpongeMatrix A, uint32_t indexRound);
