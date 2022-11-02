#ifndef __MEM_ORG_H__
#define __MEM_ORG_H__

/**************************************/
/* COUNTER DESIGN */
/**************************************/
#include "base/types.hh"

#define MONO8_CTR   1 //SGX 8 byte counters - 8 per CL
#define SPLIT16_CTR 2
//Split counters - 16 per CL (2 x 64-bit Major, 16 x 16-bit Minor)
#define SPLIT32_CTR 3
//Split counters - 32 per CL (2 x 48-bit Major, 32 x 9-bit Minor)
#define SPLIT64_CTR 4
//Split counters - 64 per CL (1 x 64-bit Major, 64 x 6-bit Minor)
#define SPLIT32_CTR_v1 5
//Split counters - 32 per CL (1 x 64-bit Major, 32 x 12-bit Minor)
#define SPLIT16_CTR_v1 6
//Split counters - 16 per CL (1 x 64-bit Major, 16 x 24-bit Minor)
#define SPLIT64_CTR_v1 7
//Split counters - 64 per CL (1 x 64-bit Major, 64 x 7-bit Minor)
#define SPLIT128_CTR 8
//Split counters - 128 per CL (1 x 64-bit Major, 128 x 3-bit Minor)
#define SPLIT128_CTR_v1 9
//Split counters - 128 per CL (1 x 64-bit Major, 128 x 3.5-bit Minor)
#define SPLIT256_CTR 10
//Split counters - 256 per CL (1 x 64-bit Major, 256 x 2-bit Minor)
#define SPLIT512_CTR 11
//Split counters - 512 per CL (1 x 64-bit Major, 512 x 1-bit Minor)

#define SPLIT128_FULL_DUAL 12
//Split counters - 128 per CL
//(1 x 53-bit Major, 2 x 10-bit Medium Counter, 2 x 64 x 3-bit Minor)
#define SPLIT128_UNC_DUAL 13
//Split counters - 128 per CL
//(1 x 53-bit Major, 2 x 10-bit Medium Counter, 2 x 64 x 3-bit Minor)
//-> Starts with AZC then devolves to 2-Base Uncompressed.
#define SPLIT128_UNC_7bINT_DUAL 14
//Split counters - 128 per CL
//(1 x 49-bit Major, 2 x 7-bit Medium Counter, 2 x 64 x 3-bit Minor)
//-> Starts with AZC then devolves to 2-Base Uncompressed.

/**************************************/

#define GB    (1024*1024*1024)
#define MB    (1024*1024)
#define KB    (1024)

#define ENCR_DELAY 40 //cycles

#define CACHE_LINE 64 // Bytes
#define CACHE_LINE_SIZE 64 // Bytes
#define MAC_SIZE   8  // Bytes




namespace gem5 {
  namespace memory {
    typedef struct
    {
      Addr paddr;
      int mtreeLevel;
      long long int entryNum;
    } CtrMtreeEntry;

  typedef struct
  {
    Addr startPAddr;
    long long int memSize; // Data size in Byte
    long long int ctrStoreSize;
    long long int MACStoreSize;
    long long int mtreeStoreSize;

    long long int numMtreeRoot;
    // BMT:
    // 8 bytes is one root.
    long long int numMtreeLevels;

    Addr*   mtreeLevelsStartAddr;
    // Array holds the start byte_addr of each Mtree Level [0-num_Mtree_levels]
    unsigned long long*   mtreeLevelSize;
    // Array holds the sizes for each of the levels

  } MemOrg;


    void initMemOrg(Addr start_paddr, unsigned long long mem_size, MemOrg * mem_org, bool is_OOP);
//Function to return the (byte) address of the MAC,
//given cacheline physical (byte) address
    Addr getMACAddr( Addr cacheline_paddr, MemOrg * mem_org );

    //Function to return the (byte) address of the Counter,
    //given cacheline physical (byte) address
    CtrMtreeEntry getCounterAddr( Addr cacheline_paddr, MemOrg * mem_org );

    //Function to return the (byte) address of the parent MTree entry,
    //given Counter/Mtree entry (byte) address
    CtrMtreeEntry getMtreeEntry( CtrMtreeEntry child , MemOrg * mem_org );
    CtrMtreeEntry getMtreeEvictParent( Addr child_paddr, MemOrg * mem_org );

    //Function returns whether address belongs to data pages or not
    int getPartition(Addr paddr, MemOrg * mem_org);

    //Helper function to calculate log to base 2
    unsigned int logBase2(unsigned int new_value);

    //Get the num_children and physical address of first child,
    //for a given counter
    Addr getCtrChild(Addr ctr_paddr, MemOrg * mem_org, int* num_children);
 } //namespace memory
} //namespace gem5






#endif
