#include <stdio.h>
#include <math.h>
/**
Tevin Otieno
Computer Organization
HW6
*/

//Struct for a split up address
typedef struct splitAddr_t {
  int address;
  int setNum;
  int tag;
  int hit;
} splitAddr;

//struct for a cache line
typedef struct cacheLine_t {
  int valid;
  int tag;
  int counter;
} cacheLine;

//set is defined twice for the sake of findMaxCounter()

typedef struct set_t {
	int setNum;
	cacheLine cacheLinesInSet[];
}set;


//Init. methods
void cacheSim(int cacheSize, int cacheBlockSize, int blocksPerSet);
splitAddr splitAddrFunc(int addr, int offsetSize, int setSize);

//Init global data.
int addrList[39] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72,
  76, 80, 0, 4, 8, 12, 16, 71, 3, 41, 81, 39, 38, 71, 15, 39, 11, 51, 57, 41};

int addrLen = 39;

void cacheSim(int cacheSize, int cacheBlockSize, int blocksPerSet) {
	//struct for a complete cache (array of sets)
	//For now commented out and defined outside the fucntion with default size set at
	
	typedef struct set_t {
		int setNum;
		cacheLine cacheLinesInSet[blocksPerSet];
	}set;
	

	//Initialize cache
	int numCacheLines = cacheSize/cacheBlockSize;	//Total Number of Cache Lines
	int numSets = numCacheLines/blocksPerSet;		//Total Number of sets
	set cache[numSets];								//Cache is an array of sets

	//Size of offset, set and tag from addresses
	int offsetSize = log2(cacheBlockSize);
	int setSize = log2(numSets);
	int tagSize = 32 - offsetSize - setSize;


	//Initialize cache with set numbers and counter
	printf("Initial Cache:\n");
	printf("Set\tTag\tCounter\t\n");

	int c1;
	for(c1 = 0; c1 < numSets; c1++) {
		//Assign set number
		cache[c1].setNum = c1;

		//initialize counters for FIFO
		/*
		How the counters work
		In each set, topmost is initialized to highest index in the set (i.e. in a 2 way set associative, it'd be 1)
		keep decrementing to 0
		Everytime you don't have a hit in the set, you evict the highest counter in the set, set that counter to 0 and increment the rest
		when you have a hit, do nothing
		*/
		int c2;
		for(c2 = blocksPerSet - 1; c2 >= 0; c2--) {
			cache[c1].cacheLinesInSet[blocksPerSet - (c2 + 1)].counter = c2;
		}

		//Initializes tags and valids
		//Prints initial state of set
		//Should all be 0 (except counters)
		int c8;
		for(c8 = 0; c8 < blocksPerSet; c8++) {
			cache[c1].cacheLinesInSet[c8].valid = 0;
			cache[c1].cacheLinesInSet[c8].tag = 0;
			printf("%d\t%d\t%d\n",cache[c1].setNum, cache[c1].cacheLinesInSet[c8].tag, cache[c1].cacheLinesInSet[c8].counter);
		}
		printf("\n");
	}
	//Print Header thingy
	printf("Address\tSet\tTag\t\n");
	//Go through addresses and assign Hits and misses
	int c3;
	for(c3 = 0; c3 < addrLen; c3++) {
		//Print Header thingy
		//Split up current address to a struct with a set number and a tag
		splitAddr curAddr = splitAddrFunc(addrList[c3], offsetSize, setSize);

		//Find the set to work with Using the current addresses set #
		set* curSet = &(cache[curAddr.setNum]);

		//Compare tags in set to check for hits
		int c4;
		for(c4 = 0; c4 < blocksPerSet; c4++) {
			//current Cache Line is a pointer to the cache
			cacheLine* curCacheLine = &(curSet->cacheLinesInSet[c4]);

			//is it a hit?
			//TODO: decide do we print out the cache's tag&set or the addresses
			if((curCacheLine->tag == curAddr.tag) && (curCacheLine->valid > 0)) {
				//Update hit for the current address to 1
				curAddr.hit = 1;
				//Print out hit prompt.
				//No need to update cache because if it's a hit, it exists already
				printf("%d\t %d\t %d\t - HIT\n", addrList[c3], curSet->setNum, curCacheLine->tag);
			}
			//Print statments for debugging
			//Prints out address, tag and set of miss
		}

		//If miss, then you want to add the current Address to the cache.
		//Also print out relevant miss dialogue
		if(curAddr.hit == 0) {
			//Sets the current cache line (the one you're going to evict) to the cache line in the set with the highest counter
			//Find max counter also increments counters in the entire set for the next eviction
			cacheLine* curCacheLine = &(curSet->cacheLinesInSet[findMaxCounter(curSet, blocksPerSet)]);
			//Update tag to tag of address (adding address to cache)
			curCacheLine->tag = curAddr.tag;

			//update counter to last inserted
			curCacheLine->counter = 0;

			//update cache to valid
			curCacheLine->valid = 1;

			//print chyeah
			//This way, we'll know if it's updating properly
			printf("%d\t %d\t %d\t - MISS\n", addrList[c3], curSet->setNum, curCacheLine->tag);
		}
	}

	//Print final cahe
	printf("\nFinal Cache:\n");
	printf("Set\tTag\tCounter\t\n");

	//Outer Loop (for the sets)
	int c6;
	for(c6 = 0; c6 < numSets; c6++) {
		//Inner loop
		int c7;
		for(c7 = 0; c7 < blocksPerSet; c7++) {
			printf("%d\t%d\t%d\n",cache[c6].setNum, cache[c6].cacheLinesInSet[c7].tag, cache[c6].cacheLinesInSet[c7].counter);
		}

		printf("\n");
	}
	

}

//Function for splitting addresses to a set number and a tag
splitAddr splitAddrFunc(int addr, int offsetSize, int setSize) {
	//splitAddr to return
	splitAddr result;

	//address field is the address
	result.address = addr;

	//hit always initialized to 0
	result.hit = 0;

	//setNum is the address % total number of sets
	result.setNum = (addr >> offsetSize) & ((1 << setSize) - 1);

	//tag is address shifted right by size of set and offset in address
	result.tag = addr >> (setSize + offsetSize);

	return result;
}

//Function for updating counters in a set for the FIFO eviction policy
//The highest counter value is always going to be (blocksPerSet-1)
int findMaxCounter(set* s, int b) {
	//Highest counter value (the one you're trying to find)
	int temp = b - 1;
	int index;

	int c9;
	for(c9 = 0; c9 < b; c9++) {
		//If this is the highest counter, then return this index
		//Increment anyway (to prevent same counter # error (which shouldn't happen anyway (yay nested parens)))
		if (s->cacheLinesInSet[c9].counter == temp) {
			index = c9;
			s->cacheLinesInSet[c9].counter++;
		}
		//If this isn't the highest counter, increment this one and the loop goes on
		else {
			s->cacheLinesInSet[c9].counter++;
		}
	}
	return index;
}

int main() {
  
  printf("Test run 1\n");
  cacheSim(128, 8, 1);
  
  printf("Test run 2\n");
  cacheSim(64, 8, 2);

  printf("Test run 3\n");
  cacheSim(128, 16, 1);
  

  printf("Test run 4\n");
  cacheSim(64, 8, 8);
 
  return 0;
  
}
