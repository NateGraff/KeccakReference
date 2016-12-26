/*
 * Copyright 2016 Nathaniel Graff
 */

#pragma once

#include <stdint.h>

#include "KeccakSponge.h"

typedef uint8_t BitSequence;
typedef uint64_t DataLength;

typedef SpongeReturn HashReturn;
typedef SpongeState HashState;

/**
  * Function to initialize the state of the Keccak[r, c] sponge function.
  * The rate r and capacity c values are determined from @a hashBitLen.
  * @param  state       Pointer to the state of the sponge function to be initialized.
  * @param  hashBitLen  The desired number of output bits, 
  *                     or 0 for Keccak[] with default parameters
  *                     and arbitrarily-long output.
  * @pre    The value of hashBitLen must be one of 0, 224, 256, 384 and 512.
  * @return SUCCESS if successful, BAD_HASHLEN if the value of hashBitLen is incorrect.
  */
HashReturn Init(HashState * state, uint32_t hashBitLen);

/**
  * Function to give input data for the sponge function to absorb.
  * @param  state       Pointer to the state of the sponge function initialized by Init().
  * @param  data        Pointer to the input data. 
  *                     When @a dataBitLen is not a multiple of 8, the last bits of data must be
  *                     in the most significant bits of the last byte.
  * @param  dataBitLen  The number of input bits provided in the input data.
  * @pre    In the previous call to Absorb(), dataBitLen was a multiple of 8.
  * @return SUCCESS if successful, FAIL otherwise.
  */
HashReturn Update(HashState * state, const BitSequence * data, DataLength dataBitLen);

/**
  * Function to squeeze output data from the sponge function.
  * If @a hashBitLen was not 0 in the call to Init(), the number of output bits is equal to @a hashBitLen.
  * If @a hashBitLen was 0 in the call to Init(), the output bits must be extracted using the Squeeze() function.
  * @param  state       Pointer to the state of the sponge function initialized by Init().
  * @param  hashVal     Pointer to the buffer where to store the output data.
  * @return SUCCESS if successful, FAIL otherwise.
  */
HashReturn Final(HashState * state, BitSequence * hashVal);

/**
  * Function to compute a hash using the Keccak[r, c] sponge function.
  * The rate r and capacity c values are determined from @a hashBitLen.
  * @param  hashBitLen  The desired number of output bits.
  * @param  data        Pointer to the input data. 
  *                     When @a dataBitLen is not a multiple of 8, the last bits of data must be
  *                     in the most significant bits of the last byte.
  * @param  dataBitLen  The number of input bits provided in the input data.
  * @param  hashVal     Pointer to the buffer where to store the output data.
  * @pre    The value of hashBitLen must be one of 224, 256, 384 and 512.
  * @return SUCCESS if successful, BAD_HASHLEN if the value of hashBitLen is incorrect.
  */
HashReturn Hash(uint32_t hashBitLen, const BitSequence * data, DataLength dataBitLen, BitSequence * hashVal);
