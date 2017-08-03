
#include <stdlib.h>
#include <stdio.h>

// a SET is made up of n blocks, in direct mapped there is 1 set per block.
// in 2 way, it is made up of 1 set per 2 blocks
// in 4 way, it is made up of 1 set per 4 blocks
// and so on.
// For Full-way assoc, there is 1 set per block. (numBlock sets)
// eat set is allocated cacheLineSize 
//
// for example, for a direct mapped 128 byte cache and an 8 byte cache
//  line size, we will  have 16 sets with 1 block each.
//
// The same CPU but with 2-way associative is 8 sets with 2 blocks each.
//
// The same CPU but with fully associative is 1 set with 16 blocks each.
// 
// A BLOCK has a valid bit and its tag. It is the thing that shows up
// In the 'Cache contents'
// It must also have a timestamp to keep track of which one was first in and first out.
// we will have a global variable to keep track of the amount of writes.
// We keep track of the block's set by having a set of blocks.
//
// Given a fixed size of cache and cache line, there will always be the same amount of blocks.
//  the only thing that changes is how they are split up with sets.


struct block { int valid; int tag; int time; };
struct set { struct block *blocks[50]; int size; };
struct set *cache[5000];

// information about the CPU

int byte;
int n;
int cacheLineSize;

int numBlocks;
int numSets;
int globTime;

int byteLog;
int cacheLineSizeLog;


unsigned int input[39] =  {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 0, 4, 8, 12, 16, 71, 3, 41, 81, 39, 38, 71, 15, 39, 11, 51, 57, 41};

int inputLen;


void freeCache();
void printCache();
int isHitOrMiss(int tag, int index);
void initializeCache();
int main() {

	initializeCache();

	for(int count = 0; count!=inputLen; count++) {
		int tag = input[count]/(byte/n); // bitshift in order to get tag
		// index is log2(bytes/cachelinesize\n) bits large
		// tag is log2(bytes/cachelinesize/
		int index = (input[count]/cacheLineSize)%((byte/cacheLineSize)/n);
		int offset = input[count]%cacheLineSize; 
		int answer = isHitOrMiss(tag, index);
		if(answer)
			printf("address(%d): HIT   (Tag/Set#/Offset: %d/%d/%d)\n", input[count], tag, index, offset);
		else
			printf("address(%d): MISS  (Tag/Set#/Offset: %d/%d/%d)\n", input[count], tag, index, offset);
	}

	printCache();
	freeCache();
}

void initializeCache() {

	printf("How large is the cache, in bytes? ");
	scanf("%d", &byte);
	printf("How large is the cache line size, in bytes? ");
	scanf("%d", &cacheLineSize);
	printf("Direct-mapped cache? [y/N]: ");
	char c;
	scanf(" %c", &c);
	if(c == 'y') {
		n = 1;
	}
	else {
		printf("Fully associative cache? [y/N]: ");
		scanf(" %c", &c);
		if(c == 'y')
			n = byte/cacheLineSize;
		else {
			printf("Which way associative? ");
			scanf("%d", &n);
		}
	}
	// initialize all of the information about the CPU here	
	//byte = 64;
	//n = 4;
	//cacheLineSize = 8;

	numBlocks = byte/cacheLineSize;
	numSets = numBlocks/n;

	globTime = 0;

	byteLog = 7;
	cacheLineSizeLog = 3;

	inputLen = 39;

	for(int count = 0; count != numSets; count++) {
		cache[count] =  malloc(sizeof(struct set));
		cache[count]->size = n;
		//*(cache+count).blocks = (block *)malloc(sizeof(block));
		// there are a certain number of sets, and each set has a certain number of blocks.
		// We must now instantiate the blocks as well.
		for(int counter = 0; counter!=n; counter++) {
			cache[count]->blocks[counter] = malloc(sizeof(struct block));
			cache[count]->blocks[counter]->valid = 0;
			cache[count]->blocks[counter]->tag = -1;
			cache[count]->blocks[counter]->time = 0;
		}
		// we have now instantiated the correct amount of blocks per set as well as
		// set their values. (Valid = 0, Tag = -1, time = 0)
	}
}

void printCache() {
	printf("Cache contents (for each cache row index):\n");
	for(int count = 0; count!=numSets; count++) {
		for(int counter = 0; counter != cache[count]->size; counter++) {
			char trueStr[10] = "True";
			char falseStr[10] = "False";
			char *str;
			if(cache[count]->blocks[counter]->valid)
				str = trueStr;
			else
				str = falseStr;
			if(cache[count]->blocks[counter]->tag == -1) {
				printf("%d, Valid: %s; Tag: - (Set #: %d)\n",
						counter + count*n, str, count);
			}
			else
				printf("%d, Valid: %s; Tag: %d (Set #: %d)\n", 
						counter + count*n, str,
						cache[count]->blocks[counter]->tag, count);
		}
	}
}


// FIX THIS LATER, INITIALIZE SEEMS TO BE WORKING
void freeCache() {
	for(int count = 0; count!=numSets; count++) {
		for(int counter = 0; counter!=numBlocks/numSets; counter++) {
			free(cache[count]->blocks[counter]);
		}
		free(cache[count]);
	}
}

int isHitOrMiss(int tag, int index) {
	int isHit = 0; // start with false
	int rowIdx = 0;
	// index is set number.

	for(rowIdx = 0; rowIdx < n; rowIdx++) {
		if( cache[index]->blocks[rowIdx]->valid && cache[index]->blocks[rowIdx]->tag == tag) {
			isHit = 1;
			break;
		}
	}
	if(isHit)
		return 1; // returns true

	// now do the miss/evict thing.
	// search cache line with valid field == 0
	for(rowIdx = 0; rowIdx < n; rowIdx ++) {
		if( cache[index]->blocks[rowIdx]->valid == 0) {
			// this is a standard miss, no eviction necessary
			cache[index]->blocks[rowIdx]->valid = 1;
			cache[index]->blocks[rowIdx]->tag = tag;
			cache[index]->blocks[rowIdx]->time = globTime;
			globTime++;
			return isHit;
			//break;
		}
	}

	// if we didn't find a cache line with a valid field false, then evict
	if(rowIdx >= n) { // if it failed to find invalid cache line
		// find the globTime that is the least
		// and then replace the values in that specific block
		int hold = rowIdx;
		int leastTime = globTime;
		for(rowIdx = 0; rowIdx < n; rowIdx++) {
			if(cache[index]->blocks[rowIdx]->time < leastTime) {
				leastTime = cache[index]->blocks[rowIdx]->time;
				hold = rowIdx;
			}
		}
		// now hold holds the first in.
		cache[index]->blocks[hold]->valid = 1;
		cache[index]->blocks[hold]->tag = tag;
		cache[index]->blocks[hold]->time = globTime;
		globTime++;
		return isHit;
	}
	return isHit;
}
