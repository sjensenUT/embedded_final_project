#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <assert.h>
#include <stdbool.h>

#include "../timing-harness/harness.h"

#define MAX_LINE_LENGTH 4096
#define MEASURES_PER_BB 5

uint8_t parse_hex_digit(char c) {
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 0xa;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 0xa;
  return c - '0';
}

uint8_t *hex2bin(char *hex) {
  size_t len = strlen(hex);
  assert(len % 2 == 0);
  uint8_t *bin = (uint8_t *)malloc(len / 2);
  size_t i;
  for (i = 0; i  < len/2; i++) {
    uint8_t hi = parse_hex_digit(hex[i*2]);
    uint8_t lo = parse_hex_digit(hex[i*2 + 1]);
    bin[i] = hi * 16 + lo;
  }
  return bin;
}

void check_shm_fd(int fd)
{
  if (fd == -1)
  {
    printf("Something went wrong with SHM FD's\n");
    exit(-1);
  }
}

/*

argv[1]: Input file name with basic blocks, e.g. skl.csv
argv[2]: Output file name with basic blocks and timing numbers, e.g. output.scv
argv[3]: -n flag (optional)
argv[4]: Number of basic blocks to measure, default entire input file.

*/
int main (int argc, char** argv)
{

  if (argc != 3 && argc != 5)
  {
    printf("Usage: bb_measure [input filename] [output filename] (-n [iterations])\n");
    exit(-1);
  }

  char* ifname = argv[1];
  char* ofname = argv[2];

  FILE* ifile = fopen(ifname, "r");
  FILE* ofile = fopen(ofname, "w");
  if (!ifile || !ofile)
  {
    printf("Error opening file(s).\n");
    fclose(ifile);
    fclose(ofile);
    exit(-1);
  }

  int numIters = -1;
  if (argc == 5 && strcmp(argv[3], "-n") == 0)
  {
    numIters = atoi(argv[4]);
  }

  char *bb_hex, *bb_bin;
  char line[MAX_LINE_LENGTH];  

  int unrollFactorA, unrollFactorB;
  int i = 0;
  int failures  = 0;
  int successes = 0;

  while ( fscanf(ifile, "%s\n", line) == 1 )
  {
    // See if we've run enough iterations
    i++;
    if (numIters > -1 && i > numIters) break;

    // Ignore empty basic blocks
    if (line[0] == ',')
    {
      continue;
    }

    // Read the BB (everything until the ",")
    bb_hex = strtok(line, ",");

    // Determine the unroll factors A and B according to the methodology
    // discussed in the paper.
    int bb_length = strlen(bb_hex) / 2; // Two nibble characters per byte

    if (bb_length < 100) {
      unrollFactorA = 100;
      unrollFactorB = 200;
    } else if (bb_length < 200) {
      unrollFactorA = 50;
      unrollFactorB = 100;
    } else {
      unrollFactorA = 16;
      unrollFactorB = 32;
    }
    
    bb_bin = hex2bin(bb_hex);
    
    int l1_read_supported, l1_write_supported, icache_supported;

    int shm_fd;
    int minA = -1;
    int minB = -1;

    // This whole thing executes 5 times (MEASURES_PER_BB). See "Environment Variance" section
    // in the paper
    bool measurementFailed = false;
    int k;
    for (k = 0; k < MEASURES_PER_BB; k++)
    {

        // Now run the measurement with both unroll factors 10 times. (10 = HARNESS_ITERS)
        // This loop is handled within the measure function.
        // For some reason you need to make a new fd each time
        shm_fd = create_shm_fd("shm-path");  
        check_shm_fd(shm_fd); 
        struct pmc_counters *countersA = measure(
            bb_bin, bb_length, unrollFactorA,
            &l1_read_supported, &l1_write_supported, &icache_supported, shm_fd);

        shm_fd = create_shm_fd("shm-path");
        check_shm_fd(shm_fd);
        struct pmc_counters *countersB = measure(
            bb_bin, bb_length, unrollFactorB,
            &l1_read_supported, &l1_write_supported, &icache_supported, shm_fd); 
        

        if (!countersA || !countersB)
        {
          measurementFailed = true;
          break;
        }

        // Calculate the shortest execution time for A and B
        // Starting at 1 to mimic the behavior in test.c
        int j;
        for (j = 1; j < HARNESS_ITERS; j++)
        {
          int cyc_a = countersA[j].core_cyc;
          int cyc_b = countersB[j].core_cyc;
          if ( cyc_a > 0 && (minA == -1 || cyc_a < minA) ) minA = cyc_a;
          if ( cyc_b > 0 && (minB == -1 || cyc_b < minB) ) minB = cyc_b;
        }

        // Invalid measurement if minB < minA
        if (minB <= minA) 
        {
          measurementFailed = true;
          break;
        }
   
    }

    // Free the memory space that was created by hex2bin
    free(bb_bin);
    
    // If one of the measurements fails, discard the entire basic block.
    // Keep track of how many fail for our own bookkeeping.
    if (measurementFailed) {
      failures++;
      continue;
    } else {
      successes++;
    }
   
    // The estimated cycle count for the basic block is now the difference of 
    // the minimums. Note that we do not divide by the difference in unroll 
    // factor. I'm not sure why not, but looking at the bhive data, they 
    // just recorded the throughput of the unrolled BB.
    float bb_estimate = minB - minA;
    
    // Write the data to the output file.
    fprintf(ofile, "%s,%f\n", bb_hex, bb_estimate);
    // Print the first few outputs to stdout as well (for debugging)
    if (i < 10)
    {
      printf("\n\n%s,%f\n\n\n", bb_hex, bb_estimate);
    } else if (i == 11) {
      printf("...\n\n");
    }
    
  }

  fclose(ifile);
  fclose(ofile);

  // Print successes/failures
  printf("Successfully collected BB performance data for %d out of %d blocks.\n",
         successes, successes + failures);

  return 0;

}
