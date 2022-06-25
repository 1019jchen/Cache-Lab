/*
 *Name: James Chen
 *ID: jyc8938
 */
#include "lab3.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>

//Cache parameters struct
typedef struct{
  int s; //2^s cache set
  int S; //number of sets
  int E; //cache lines in a set
  int b; //parameter used for getting B
  int B; //B = 2^b (bytes per cache block)
} CacheParameter;

typedef unsigned long long int address;

//Cache Line struct
//contains 64 bit memory address, time access tracker, valid bit
typedef struct{
  address tag; //64 bit memory address
  int lru; //least-recently used
  int valid;
} CacheLine;

// Cache set struct
typedef struct{
  CacheLine *lines;
} Cacheset;

// Cache struct
typedef struct{
  Cacheset *sets;
} Cache;

int hitCount = 0;
int missCount = 0;
int evictionCount = 0;
unsigned long long int lru_counter = 1;
Cache cache;

void displayUsage(char arg[]){
  printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
  printf("Options : \n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("-b <num> Number of block offset bits.\n");
  //Fill in remaining print statements
  printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", arg);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", arg);
    exit(0);
}


/*
 *accessData - access data  at memory address addr.
 * Increase hit_count if already in cache
 * Increase miss_count if not in cache
 * Increase eviction_count if line is evicted
 * Properly put in or evict elements of the cache
 */

void accessData(address addr, int S, int s, int b, int E, int verboseFlag){
  unsigned long long int eviction_lru = ULONG_MAX;
  unsigned int eviction_line = 0;//index of a line to be evicted
  address setIndexMask = (address)(S-1);//used to take only the S set bits
  address set_index = (addr >> b) & setIndexMask;//store set bits into set_index
  address tag = addr >> (s+b);//store tag bits into tag
  for (int i=0; i<E; ++i){//iterate through every line of the set
    if (cache.sets[set_index].lines[i].valid==1){//check if line is valid
      if (cache.sets[set_index].lines[i].tag == tag){//check if tag matches
        cache.sets[set_index].lines[i].lru = lru_counter++;//increase lru_counter in the line
        hitCount++;//increase number of hits
        if (verboseFlag){
          printf("hit ");//print a hit
        }
        return;
      }
    }
  }
  missCount++;//if we did not previously hit, then this is a miss
  if (verboseFlag){
          printf("miss ");//print a miss
        }
  for (int i=0; i<E; ++i){//iterate through every line of the set again if this is a miss
    if (eviction_lru>cache.sets[set_index].lines[i].lru){//Find the lowest value of lru in the set
      eviction_line = i;
      eviction_lru = cache.sets[set_index].lines[i].lru;
    }
  }
//evict the line with the lowest lru value if it is valid
  if (cache.sets[set_index].lines[eviction_line].valid){
    if (verboseFlag){
          printf("eviction ");
        }
    evictionCount++;//increase eviction counts
  }
//putting in the new line if we had a miss
  cache.sets[set_index].lines[eviction_line].valid = 1;
  cache.sets[set_index].lines[eviction_line].tag = tag;
  cache.sets[set_index].lines[eviction_line].lru = lru_counter++;

}

int main(int argc, char** argv)
{
  int c;
  CacheParameter cacheParam;
  int helpFlag = 0; // -h: switch to help mode
  int verboseFlag = 0; // -v: switch to verbose mode
  int sFlag = 0;
  int EFlag = 0;
  int bFlag = 0;
  int tFlag = 0;
  char *tFile;
  /* read argv with getopt()
    */
  while((c = getopt(argc, argv, "s:E:b:t:vh"))!=-1){
    switch(c){
      case 'h':
        //optional flag for displaying usage information
        helpFlag=1;
        break;
      case 'v':
        //optional flag for displaying trace information
        verboseFlag = 1;
        break;
      case 's':
        // 2^s cache sets
        cacheParam.s = (atoi(optarg)); //a to i: ascii to integer
        sFlag = 1;
        break;
      case 'E':
        // Associativity: lines per set
        cacheParam.E = (atoi(optarg));
        EFlag = 1;
        break;
      case 'b':
        //block bits (2^b block size in total)
        cacheParam.b = (atoi(optarg));
        bFlag = 1;
        break;
      case 't':
        // Valgrind trace
        tFile = optarg;
        tFlag = 1;
        break;
      default:
        displayUsage(argv[0]);
        break;
    }
  }
  /*Setting the -h flag causes the program to print the usage information
  * It also forces the program to translate regardless of any other flags set
  */
  if (helpFlag == 1){
    displayUsage(argv[0]);
    exit(EXIT_SUCCESS); //successful termination
  }
  if (sFlag==0||EFlag==0||bFlag==0||tFlag==0){
    printf("./csim:Missing required command line argument\n");
    displayUsage(argv[0]);
    exit(EXIT_FAILURE); //unsuccessful termination
  }
  FILE *fp; //FILE pointer to open
  fp = fopen(tFile, "r"); //read valgrind trace file
  if (fp==NULL){
   printf("%s: No such file or directory", tFile);
   exit(EXIT_FAILURE); //unsuccessful termination
  }
  cacheParam.S = (int)pow(2,cacheParam.s);
  cacheParam.B = (int)pow(2,cacheParam.b);
  //Dynamically allocate memory for cache
  cache.sets = (Cacheset*)malloc(cacheParam.S * sizeof(Cacheset)); //allocate memory for sets
  //allocate size of line * number of lines per set
  for (int i=0; i<cacheParam.S; i++){
    cache.sets[i].lines = (CacheLine*)malloc(sizeof(CacheLine)*cacheParam.E);
    for (int j=0; j<cacheParam.E; j++){
      cache.sets[i].lines[j].lru = 0;
      cache.sets[i].lines[j].valid = 0;
      cache.sets[i].lines[j].tag = 0;
    }
  }
  
  

  //count hit, miss, and eviction

    char trace_cmd;
    address address;
    int size;
//void accessData(address addr, int hitCount, int missCount, int evictionCount, unsigned long long int lru_counter, int s, int b, int E, Cache cache, CacheParameter param, int verboseFlag)
    while (fscanf(fp, " %c %llx,%d", &trace_cmd, &address, &size) == 3) {
        switch(trace_cmd) {//check the character in the valgrind trace
            case 'L': //access data once if character is L
              if (verboseFlag){
                printf("%c %llx, %d ", trace_cmd, address, size);
              }
              accessData(address, cacheParam.S, cacheParam.s, cacheParam.b, cacheParam.E, verboseFlag);
	      if (verboseFlag){
		printf("\n");
		} 
              break;
            case 'S'://access data once if character is S 
              if (verboseFlag){
                printf("%c %llx, %d ", trace_cmd, address, size);
              }
              accessData(address, cacheParam.S, cacheParam.s, cacheParam.b, cacheParam.E, verboseFlag);
		if (verboseFlag){
		printf("\n");
		}  
              break;
            case 'M': //access data TWICE if character is M
              if (verboseFlag){
                printf("%c %llx, %d ", trace_cmd, address, size);
              }
              accessData(address, cacheParam.S, cacheParam.s, cacheParam.b, cacheParam.E, verboseFlag); 
              accessData(address, cacheParam.S, cacheParam.s, cacheParam.b, cacheParam.E, verboseFlag); 
		if(verboseFlag){
		printf("\n");
		}
              break;
            default: 
              break;
          }
        }
  //close the file pointer, no longer needed
  fclose(fp);
  //free the memory
    for (int i = 0; i < cacheParam.S; i++) {
        free(cache.sets[i].lines);
    }
    free(cache.sets);
//use the printSummary function
  printSummary(hitCount, missCount, evictionCount);
  return 0;
}
