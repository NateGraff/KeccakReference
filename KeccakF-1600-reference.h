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

// Initialization
void KeccakInitialize(unsigned char * state);

// Absorb and Permute
void KeccakAbsorb(unsigned char *state, const unsigned char *data, unsigned int laneCount);
void KeccakPermutation(unsigned char *state);

// Squeezing
void KeccakExtract(const unsigned char *state, unsigned char *data, unsigned int laneCount);
