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
#define SIZE_OF_HARNESS_MEM (4096 * 3)
#define REF_COMPARE_THRESHOLD 0.2
#define MAX_BB_LENGTH 600

const int ITERS_TO_SKIP[2] = {1001, 6621};

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

int create_shm_fd_2(char *path) {
  int fd = shm_open(path, O_RDWR|O_CREAT, 0777);
  ftruncate(fd, SIZE_OF_HARNESS_MEM);
  return fd;
}

int shm_fd_a, shm_fd_b;

void open_shms()
{
  shm_fd_a = create_shm_fd_2("shm-path-a");
  shm_fd_b = create_shm_fd_2("shm-path-b");
}

void close_shms()
{
  shm_unlink("shm-path-a");
  shm_unlink("shm-path-b");
}

void reinit_shms()
{
  close_shms();
  open_shms();  
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
  float scaleFactor;
  int i = 0;
  int failures  = 0;
  int successes = 0;

  open_shms();

  //int iters_to_skip_size = sizeof(ITERS_TO_SKIP)/sizeof(int);

  while ( fscanf(ifile, "%s\n", line) == 1 )
  {
    // See if we've run enough iterations
    i++;
    if (numIters > -1 && i > numIters) break;

    /*// See if we should skip this BB (numbers hard-coded for convenience)
    int w;
    bool skipThisIter = false;
    for (w = 0; w < iters_to_skip_size; w++) {
      if (i == ITERS_TO_SKIP[w]) {
        skipThisIter = true;
        break;
      }
    }
    
    if (skipThisIter) {
      printf("Skipping iteration %d\n", i);
      continue;
    }*/

    // Ignore empty basic blocks
    if (line[0] == ',')
    {
      printf("Empty basic block on iter %d\n", i);
      continue;
    }

    // Read the BB (everything until the ",")
    bb_hex = strtok(line, ",");
    float ref_val = atof(strtok(NULL, "\n"));

    // Determine the unroll factors A and B according to the methodology
    // discussed in the paper.
    int bb_length = strlen(bb_hex) / 2; // Two nibble characters per byte

    // For some reason very large BB's are breaking the program. Skip them
    if (bb_length > MAX_BB_LENGTH) continue;

    
    if (bb_length < 100) {
      unrollFactorA = 100;
      unrollFactorB = 200;
      scaleFactor = 1.0;
    } else if (bb_length < 200) {
      unrollFactorA = 50;
      unrollFactorB = 100;
      scaleFactor = 2.0;
    } else {
      unrollFactorA = 16;
      unrollFactorB = 32;
      scaleFactor = 100.0/16.0;
    }
    // TEMP
    //unrollFactorA = 100;
    //unrollFactorB = 200;
    
    bb_bin = hex2bin(bb_hex);
    
    int l1_read_supported, l1_write_supported, icache_supported;

    int minA = -1;
    int minB = -1;
    bool no_cache_misses_A = false;
    bool no_cache_misses_B = false;

    // This whole thing executes 5 times (MEASURES_PER_BB). See "Environment Variance" section
    // in the paper
    bool measurementFailed = false;
    int k;
    for (k = 0; k < MEASURES_PER_BB; k++)
    {

        // Now run the measurement with both unroll factors 10 times. (10 = HARNESS_ITERS)
        // This loop is handled within the measure function.
        struct pmc_counters *countersA = measure(
            bb_bin, bb_length, unrollFactorA,
            &l1_read_supported, &l1_write_supported, &icache_supported, shm_fd_a);

        struct pmc_counters *countersB = measure(
            bb_bin, bb_length, unrollFactorB,
            &l1_read_supported, &l1_write_supported, &icache_supported, shm_fd_b); 
        

        if (!countersA || !countersB)
        {
          if (!countersA) {
             printf("Measurement A failed on iter %d (bb_length = %d)\n", i, bb_length);
          } 
          if (!countersB) {
             printf("Measurement B failed on iter %d (bb_length = %d)\n", i, bb_length);
          }
          // Try to close an re-open the SHMs in hope that the next BB will succeed.
          //reinit_shms();
          //measurementFailed = true;
          //break;
          // Can't figure out how to recover from this type of failure...
          close_shms();
          fclose(ifile);
          fclose(ofile);
          exit(-1);
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
          no_cache_misses_A = no_cache_misses_A || (countersA[j].l1_read_misses == 0);
          no_cache_misses_B = no_cache_misses_B || (countersB[j].l1_read_misses == 0);
        }

        // Invalid measurement if minB < minA
        if (minB <= minA) 
        {
          printf("Invalid measurement (B faster than A) on iter %d: (%d, %d)\n", i, minA, minB);
          measurementFailed = true;
          break;
        }
   
    }

    // Free the memory space that was created by hex2bin
    free(bb_bin);
   
    // Invalid measuremenr if we never saw 0 cache misses
    if (!measurementFailed && (!no_cache_misses_A || !no_cache_misses_B)) {
      printf("Invalid measurement (iter = %d) due to cache misses.\n", i);
      measurementFailed = true;
    }

    // Invalid measurement if minB < minA
    if (!measurementFailed && (minB <= minA)) 
    {
      printf("Invalid measurement (B faster than A) on iter %d: (%d, %d)\n", i, minA, minB);
      measurementFailed = true;
    }
 
    // The estimated cycle count for the basic block is now the difference of 
    // the minimums, multiplied by the scale factor (need to confirm with BHive
    // researchers that this is what they did)
    float bb_estimate = (minB - minA) * scaleFactor;

    // Skip the sample and mark as failure if the measurement is more than 50%
    // different than the measurement file.
    float ref_compare = abs(ref_val - bb_estimate) / ref_val;
    if(!measurementFailed && (ref_compare > REF_COMPARE_THRESHOLD)) {
      //printf("Invalid measurement (iter %d): measured throughput value (%f) was much different than reference (%f)\n",
      //       i, bb_estimate, ref_val);
      //printf("Unroll factors: (%d, %d). bb_length: %d\n", unrollFactorA, unrollFactorB, bb_length);
      measurementFailed = true;
    }     
    
    // If one of the measurements fails, discard the entire basic block.
    // Keep track of how many fail for our own bookkeeping.
    if (measurementFailed) {
      failures++;
      continue;
    } else {
      successes++;
    }

    // Write the data to the output file.
    fprintf(ofile, "%s,%f\n", bb_hex, bb_estimate);
    // Print the first few outputs to stdout as well (for debugging)
    if (i < 10)
    {
      printf("%s,%f\n", bb_hex, bb_estimate);
    } else if (i == 11) {
      printf("...\n\n");
    } else if (i % 100 == 0) {
      printf("Progress: i = %d\n", i);
    }
    
  }

  close_shms();

  fclose(ifile);
  fclose(ofile);

  // Print successes/failures
  printf("Successfully collected BB throughput data for %d out of %d blocks.\n",
         successes, successes + failures);

  return 0;

}
