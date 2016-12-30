#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "KeccakF-1600-reference.h"
#include "KeccakSponge.h"
#include "KeccakNISTInterface.h"

void alignLastByteOnLSB(const unsigned char *in, unsigned char *out, unsigned int length)
{
    unsigned int lengthInBytes;

    lengthInBytes = (length+7)/8;
    memcpy(out, in, lengthInBytes);
    if ((length % 8) != 0) {
        out[lengthInBytes-1] = out[lengthInBytes-1] >> (8-(length%8));
    }
}

void generateVHDLTestVectors(uint32_t num_test)
{
	const uint32_t rate = 1024;

    FILE * keccak_in;
    FILE * keccak_code_in;
    FILE * keccak_out;
    FILE * keccak_code_out;

    keccak_in = fopen("keccak_in.txt", "w");
    keccak_code_in = fopen("keccak_code_in.txt", "w");
	keccak_out = fopen("keccak_ref_out.txt", "w");
	keccak_code_out = fopen("keccak_code_out.txt", "w");

	fprintf(keccak_in, "%d\n", num_test);		

	for(uint32_t count_test = 0; count_test < num_test; count_test++)
	{
	    uint8_t message[4000];
		uint8_t padded_message[4000];

		// Generate random messages of increasing length
		uint32_t messageLength = 100 + count_test;

		memset(&message, 0, sizeof(message));

		for(uint32_t j = 0; j < (1 + messageLength/8); j++) {
			message[j] = (char) rand();
		}

		uint8_t mask = 0xff >> (8 - messageLength % 8);

		message[messageLength/8] = message[messageLength/8] & mask;

		memset(&padded_message, 0, sizeof(padded_message));

		alignLastByteOnLSB(message, padded_message, messageLength);

		// Add Pad10*0 to message
		if (((messageLength + 1) % rate) == 0) {
			// The message is one bit short of a block, add a complete block

			padded_message[messageLength/8 ] |= 1 << (messageLength % 8);			
			
			memset(&padded_message[messageLength/8+1], 0, rate/8);
			
			//final padding
			padded_message[(messageLength + rate)/8] |= 1 << ((rate - 1) % 8);
		}
		else {
			memset(&padded_message[messageLength/8+1], 0, rate/8 - ((messageLength+7)%rate)/8);
			
			padded_message[messageLength/8 ] |= 1 << (messageLength % 8);			
			
			//final padding
			padded_message[(messageLength + (rate - (messageLength % rate)))/8 - 1] |= 1 << ((rate-1) % 8);
		}
			
		fprintf(keccak_code_in, "Message length in bits: %d\n", messageLength);
		fprintf(keccak_code_in, "Unpadded message:\n");

		for(uint32_t i = 0; i < (messageLength + 7) / 8; i++) {
			fprintf(keccak_code_in, "%02X", message[i]);
		}
		fprintf(keccak_code_in, "\n-\n");

		// Print padded message
		if (((messageLength + 1) % rate) == 0 ) {
			// The message is one bit short of a block

			for(uint32_t j = 0; j < (messageLength + 1 + rate)/rate; j++)
			{
				for(uint32_t i = 0; i < (rate/64); i++)
				{
					for(uint32_t k = 0; k < 8; k++) {
						fprintf(keccak_in,"%02X", padded_message[j * (8 * rate/64) + i * 8 + (7 - k)]);			
					}
					fprintf(keccak_in,"\n");
				}
			}

		}
		else
		{
			for(uint32_t j = 0; j < (messageLength + (rate - (messageLength % rate)))/rate; j++)
			{
				for(uint32_t i = 0; i < (rate/64); i++)
				{
					for(uint32_t k = 0; k < 8; k++) {
						fprintf(keccak_in, "%02X", padded_message[j * (8 * rate/64) + i * 8 + (7 - k)]);			
					}
					fprintf(keccak_in,"\n");
				}
			}
		}

		fprintf(keccak_in, "-\n");

		// Compute arbitrary-length output Keccak
		HashState state;
		BitSequence MD[64];

		Init(&state, 0);
		Update(&state, message, messageLength);
		Squeeze(&state, MD, 256);
		
		// Print output hashes
		for(uint32_t i = 0; i < 256/8; i++){
			fprintf(keccak_code_out, "%02x", MD[i]);
		}
		fprintf(keccak_code_out, "\n-\n");
		
		for(uint32_t j = 0; j < 256/64; j++)
			{
				for(uint32_t k = 0; k < 8; k++) {
					fprintf(keccak_out, "%02X", MD[j * 8 + (7 - k)]);
				}
				fprintf(keccak_out,"\n");

			}
			fprintf(keccak_out, "-\n");
		}

		fprintf(keccak_in, ".\n");

		fclose(keccak_in);
		fclose(keccak_code_in);
		fclose(keccak_out);
		fclose(keccak_code_out);
}

int main() {
	generateVHDLTestVectors(4);

	return 0;
}
