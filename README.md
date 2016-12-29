# Keccak Sponge Function Reference Implementation

This is a fork of the reference C implementation of the [Keccak Sponge Function Family](https://keccak.noekeon.org/)

## Goal of the Fork

The primary goal of this fork is to improve the quality of the code. I'm looking to use a C implementation of Keccak in an educational setting where students are expected to read, understand, and modify the code on their own. To make sure that this is possible with the least unnecessary mental overhead, I found it necessary to reformat, comment, and restructure the original reference implementation.

## Other Improvements

Keeping in mind that this is a cryptographic hash function, care should be taken to preserve the secrecy of the input data. Therefore, everywhere where secret data is copied into memory within the sponge function, that memory is cleared with zeroes before it goes out of scope. Users of this algorithm who wish to ensure that their input data remain secret should additionally make sure that the input data buffer in cleared before it is freed, as this algorithm does not clear it.

## License

This work is released under the MIT license (see the LICENSE file).

Copyright 2016 Nathaniel Graff