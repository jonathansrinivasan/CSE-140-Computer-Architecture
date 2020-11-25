#include "tips.h"
//Partner: Tony Varela
/* The following two functions are defined in util.c */

/* finds the highest 1 bit, and returns its position, else 0xFFFFFFFF */
unsigned int uint_log2(word w);

/* return random int from 0..x-1 */
int randomint( int x );

/*
  This function allows the lfu information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lfu information
 */
char* lfu_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lfu information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);

  return buffer;
}

/*
  This function allows the lru information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lru information
 */
char* lru_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lru information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);

  return buffer;
}

/*
  This function initializes the lfu information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lfu(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].accessCount = 0;
}

/*
  This function initializes the lru information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lru(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].lru.value = 0;
}

/*
  This is the primary function you are filling out,
  You are free to add helper functions if you need them

  @param addr 32-bit byte address
  @param data a pointer to a SINGLE word (32-bits of data)
  @param we   if we == READ, then data used to return
              information back to CPU

              if we == WRITE, then data used to
              update Cache/DRAM
*/
unsigned int findReplacementBlock (unsigned int index) {
  unsigned int i, minBlock = 0;
  for (i = 0; i < assoc; i++) {
    if (cache[index].block[i].valid == INVALID) return i; // if there's an invalid spot in the set, then just go ahead and use that one
  }
  // else, resort to either one of the policies
  if (policy == RANDOM) return randomint(assoc); // just return a random number between 0 - max number of associativity
  else if (policy == LRU) { // dont really need this else if since we're not implementing LFU, but oh well
  // we're gonna use accessCount to keep track of the LRU block. the one with the minimum value is going to be the LRU
    for (i = 0; i < assoc; i++) {
      if (cache[index].block[i].accessCount <  cache[index].block[minBlock].accessCount) minBlock = i;
    }
    return minBlock;
  }
}

void accessMemory(address addr, word* data, WriteEnable we)
{
  /* Declare variables here */
  unsigned int tag, index, indexSize, offset, offsetSize, i, replacementBlock;

  /* handle the case of no cache at all - leave this in */
  if(assoc == 0) {
    accessDRAM(addr, (byte*)data, WORD_SIZE, we);
    return;
  }

  /*
  You need to read/write between memory (via the accessDRAM() function) and
  the cache (via the cache[] global structure defined in tips.h)

  Remember to read tips.h for all the global variables that tell you the
  cache parameters

  The same code should handle random, LFU, and LRU policies. Test the policy
  variable (see tips.h) to decide which policy to execute. The LRU policy
  should be written such that no two blocks (when their valid bit is VALID)
  will ever be a candidate for replacement. In the case of a tie in the
  least number of accesses for LFU, you use the LRU information to determine
  which block to replace.

  Your cache should be able to support write-through mode (any writes to
  the cache get immediately copied to main memory also) and write-back mode
  (and writes to the cache only gets copied to main memory when the block
  is kicked out of the cache.

  Also, cache should do allocate-on-write. This means, a write operation
  will bring in an entire block if the block is not already in the cache.

  To properly work with the GUI, the code needs to tell the GUI code
  when to redraw and when to flash things. Descriptions of the animation
  functions can be found in tips.h
  */
  /* Start adding code here */

  indexSize = uint_log2(set_count);
  offsetSize = uint_log2(block_size);

  tag = addr >> (indexSize + offsetSize);
  index = (addr >> offsetSize) & ((1 << indexSize) - 1);
  offset = addr & ((1 << offsetSize) - 1); // we're masking the appropriate bits we need here; (1 << offsetSize) will give us a number with + 1 more bits than a number with offsetSize 1's, so gotta take 1 away from it

  replacementBlock = findReplacementBlock(index); // find which block to replace in the set at the index
  // first check reading
  if (we == READ) {
    //append_log("READING\n");
    for (i = 0; i < assoc; i++) {
      if (tag == cache[index].block[i].tag && cache[index].block[i].valid == VALID) { // cache read-hit
        cache[index].block[i].accessCount++; // this is for LRU use
        cache[index].block[i].lru.value = 0;
        //cache[index].block[i].valid = VALID;
        highlight_offset(index, i, offset, HIT);
        memcpy(data, cache[index].block[i].data + offset, 4); // copy the word from the specific offset index into data
        return;
      }
    }
    // if we're here, then it missed. gotta figure out which one to replace
    highlight_offset(index, replacementBlock, offset, MISS);
    highlight_block(index, replacementBlock);
    // if we're reading and we get to here, then need to check if dirty and valid to replace
    // this is for write-back
    if (cache[index].block[replacementBlock].dirty == DIRTY && cache[index].block[replacementBlock].valid == VALID) { // need to replace whatever is in there if it's not up to date with dram
      // accessdram(cache[index].block[replacementblock].tag << ((indexsize + offsetsize) + (index << offsetsize)), cache[index].block[replacementblock].data , , write);
      accessDRAM(addr, cache[index].block[replacementBlock].data, (TransferUnit)offsetSize, WRITE);
    }
    // now update the cache line content at the replacementBlock
    //accessDRAM(addr, cache[index].block[replacementBlock].data, (TransferUnit)offsetSize, READ);
    memcpy(data, cache[index].block[replacementBlock].data + offset, 4);
    cache[index].block[replacementBlock].valid = VALID;
    cache[index].block[replacementBlock].dirty = VIRGIN;
    cache[index].block[replacementBlock].tag = tag;
    cache[index].block[replacementBlock].accessCount++;
    cache[index].block[replacementBlock].lru.value = 1;
  } else { // WRITE
    //append_log("WRITING\n");
    if (memory_sync_policy == WRITE_BACK) {
      for (i = 0; i < assoc; i++) {
        if (tag == cache[index].block[i].tag && cache[index].block[i].valid == VALID) { // cache-hit
          highlight_offset(index, assoc - 1, offset, HIT);
          memcpy(cache[index].block[i].data + offset, data, 4); // write the data
          cache[index].block[i].valid = VALID;
          cache[index].block[i].dirty = DIRTY;
          cache[index].block[i].lru.value = 0;
          cache[index].block[i].accessCount++; // this is for LRU use
          return;
        }
      }

      // cache write-miss

      //replacementBlock = findReplacementBlock(index);
      highlight_offset(index, replacementBlock, offset, MISS);
      highlight_block(index, replacementBlock);
      // if (cache[index].block[replacementBlock].dirty == DIRTY) {
      //   // accessDRAM(cache[index].block[replacementBlock].tag << (index + offsetSize) + (index));
      //   //accessDRAM()
      // }
      accessDRAM(addr, cache[index].block[replacementBlock].data, (TransferUnit)offsetSize, READ);
      cache[index].block[replacementBlock].dirty = DIRTY;
      cache[index].block[replacementBlock].tag = tag;
      cache[index].block[replacementBlock].valid = VALID;
      cache[index].block[replacementBlock].lru.value = 0;
      cache[index].block[i].accessCount++; // this is for LRU use
    } else { // write-through; in write-through, each time you write to cache youre going to write to the memory as well
      for (i = 0; i < assoc; i++) { // go through each block in the current set
        if (tag == cache[index].block[i].tag && cache[index].block[i].valid == VALID) { // cache write-hit
          cache[index].block[i].valid = VALID;
          cache[index].block[i].dirty = VIRGIN;
          cache[index].block[i].accessCount++;
          cache[index].block[i].lru.value = 0;
          highlight_offset(index, i, offset, HIT);
          memcpy(cache[index].block[i].data + offset, data, 4); // write the data from CPU into the cache
          accessDRAM(addr, (byte*)cache[index].block[i].data, (TransferUnit)offsetSize, WRITE); // wrote the data from cache into memory
          return;
        }
      }

      // else, there was a cache miss if we get to here; fill in the necessary fields of the cache block
      //replacementBlock = findReplacementBlock(index);
      highlight_offset(index, replacementBlock, offset, MISS);
      highlight_block(index, replacementBlock);
      cache[index].block[replacementBlock].tag = tag;
      cache[index].block[replacementBlock].valid = VALID;
      cache[index].block[replacementBlock].dirty = VIRGIN;
      cache[index].block[replacementBlock].accessCount++;
      cache[index].block[replacementBlock].lru.value = 0;
      memcpy(cache[index].block[replacementBlock].data + offset, data, 4);
      accessDRAM(addr, (byte*)cache[index].block[replacementBlock].data, (TransferUnit)offsetSize, WRITE);
    }
  }

  /* This call to accessDRAM occurs when you modify any of the
     cache parameters. It is provided as a stop gap solution.
     At some point, ONCE YOU HAVE MORE OF YOUR CACHELOGIC IN PLACE,
     THIS LINE SHOULD BE REMOVED.
  */
  //accessDRAM(addr, (byte*)data, WORD_SIZE, we);
}