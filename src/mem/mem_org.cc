// Commentary: 
// -Functions can be used to get the address of a particular MAC,Counter, Mtree entry
// -Functions (to be written) can also return what areas memory can be usable by data
// 
//  Memory organisation is 
//  Home region: DATA : MTREE+SOME_EMPTY_SPACE : CTRs : MACs
//  OOP region:  DATA : : : MACs

// Code:
#include "mem_org.hh"

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

namespace gem5 {
  namespace memory {
    int SGX_MODE[4]={1,1,1,1};

    //Design of Counters
    int CTR_DESIGN = (MONO8_CTR);
    double CTR_SIZE   = 8;
    int CTRS_PER_MTREE = 8; //Arity below leaf-level 

    //Design of Mtree Counters -      Leaf,            Parent,    GrandParent,   GreatGrandParent, All others
    int MTREE_CTR_DESIGN_VAR[5] = {SPLIT64_CTR_v1,SPLIT64_CTR_v1,SPLIT64_CTR_v1, SPLIT64_CTR_v1   , SPLIT64_CTR_v1};

    // Num of bytes per Entry(One Entry corresponding to one CL in the child level)
    double MTREE_ENTRY_SIZE_VAR[5];

    int* MTREE_CTR_DESIGN;
    // Num of bytes per Entry(One Entry corresponding to one CL in the child level)
    double* MTREE_ENTRY_SIZE;
    int* MTREE_ARY;

/*
  int MTREE_CTR_DESIGN = (SPLIT32_CTR_v1);
  double MTREE_ENTRY_SIZE = 2;
  int MTREE_ARY  = 32; //Num counters in the current mtree line, decide the arity above leaf-level mtree.
*/

    //Helper function to calculate log to base 2
    unsigned int log_base2(unsigned int new_value)
    {
      int i;
      for (i = 0; i < 32; i++) {
        new_value >>= 1;
        if (new_value == 0)
          break;
      }
      return i;
    }


    //Constructor
    void init_memOrg(int addr_bus_num_bits, MemOrg * mem_org, bool is_OOP){

      //Initialize CTR_SIZE, CTRS_PER_MTREE
      if(CTR_DESIGN == MONO8_CTR){
        CTR_SIZE=8;
        CTRS_PER_MTREE=8; 
      }  
      else if(CTR_DESIGN == SPLIT16_CTR){
        CTR_SIZE=4;
        CTRS_PER_MTREE=8;
      }
      else if( CTR_DESIGN == SPLIT32_CTR ){
        CTR_SIZE=2;
        CTRS_PER_MTREE=16;
      }
      else if (CTR_DESIGN == SPLIT64_CTR){
        CTR_SIZE=1;
        CTRS_PER_MTREE=64;
      }
      else if (CTR_DESIGN == SPLIT128_CTR){
        CTR_SIZE=0.5;
        CTRS_PER_MTREE=128;
      }
      else if ( (CTR_DESIGN == SPLIT128_FULL_DUAL) || (CTR_DESIGN == SPLIT128_UNC_DUAL)  || (CTR_DESIGN == SPLIT128_UNC_7bINT_DUAL) ){
        CTR_SIZE=0.5;
        CTRS_PER_MTREE=128;
      }  
      else if(CTR_DESIGN == SPLIT32_CTR_v1){
        CTR_SIZE=2;
        CTRS_PER_MTREE=32;
      } 
      else if(CTR_DESIGN == SPLIT16_CTR_v1){
        CTR_SIZE=4;
        CTRS_PER_MTREE=16;
      }
      else if (CTR_DESIGN == SPLIT64_CTR_v1){
        CTR_SIZE=1;
        CTRS_PER_MTREE=64;
      }
      else if (CTR_DESIGN == SPLIT128_CTR_v1){
        CTR_SIZE=0.5;
        CTRS_PER_MTREE=128;
      }  
      else if (CTR_DESIGN == SPLIT256_CTR){
        CTR_SIZE=0.25;
        CTRS_PER_MTREE=256;
      }  
      else if (CTR_DESIGN == SPLIT512_CTR){
        CTR_SIZE=0.125;
        CTRS_PER_MTREE=512;
      }  


      
      //Initialize the Merkle Tree Design
      for(int i=0;i<5;i++){
        
        if(MTREE_CTR_DESIGN_VAR[i] == MONO8_CTR){
          MTREE_ENTRY_SIZE_VAR[i]=8;
        }  
        else if(MTREE_CTR_DESIGN_VAR[i] == SPLIT16_CTR){
          MTREE_ENTRY_SIZE_VAR[i]=4;
        }
        else if( MTREE_CTR_DESIGN_VAR[i] == SPLIT32_CTR ){
          MTREE_ENTRY_SIZE_VAR[i]=2;
        }
        else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT64_CTR){
          MTREE_ENTRY_SIZE_VAR[i]=1;
        }
        else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT128_CTR){
          MTREE_ENTRY_SIZE_VAR[i]=0.5;
        }
        else if(MTREE_CTR_DESIGN_VAR[i] == SPLIT32_CTR_v1){
          MTREE_ENTRY_SIZE_VAR[i]=2;
        } 
        else if(MTREE_CTR_DESIGN_VAR[i] == SPLIT16_CTR_v1){
          MTREE_ENTRY_SIZE_VAR[i]=4;
        }
        else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT64_CTR_v1){
          MTREE_ENTRY_SIZE_VAR[i]=1;
        }
        else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT128_CTR_v1){
          MTREE_ENTRY_SIZE_VAR[i]=0.5;
        }
        else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT256_CTR){
          MTREE_ENTRY_SIZE_VAR[i]=0.25;
        }
        else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT512_CTR){
          MTREE_ENTRY_SIZE_VAR[i]=0.125;
        }        
      }

      long long int mtree_size;
      mem_org -> memSize = ((long long int) 1 ) << addr_bus_num_bits;
      if (is_OOP)
        mem_org -> ctrStoreSize = 0; // OOP does not need to store Ctr
      else
        mem_org -> ctrStoreSize =  (((long long int) 1 ) << addr_bus_num_bits ) / CACHE_LINE * CTR_SIZE;
      mem_org -> MACStoreSize = (((long long int) 1 ) << addr_bus_num_bits ) / CACHE_LINE * MAC_SIZE;

      //Determine number of Mtree Levels
      unsigned long long num_mtree_levels = 0;
      unsigned long long level_size = mem_org -> ctrStoreSize;
      unsigned long long mtree_total_size = 0;
      mem_org -> numMtreeRoot = 0;
      if (!is_OOP){
        while(level_size > CACHE_LINE){
          int index = num_mtree_levels;
          if(index >= 4)
            index =4;

          unsigned long long num_mtree_prevlevel_elements = level_size / CACHE_LINE;
          // previous level of Merkle Tree has num_mtree_.... CacheLine
          unsigned long long prev_level_size = num_mtree_prevlevel_elements * MTREE_ENTRY_SIZE_VAR[index];

          level_size = prev_level_size;
          num_mtree_levels++;
          mtree_total_size += level_size;
        }

      mem_org -> numMtreeLevels = num_mtree_levels;

      int temp_index = (num_mtree_levels-1);
      if(temp_index >=4)
        temp_index = 4;
      //   
      mem_org -> numMtreeRoot = level_size / MTREE_ENTRY_SIZE_VAR[temp_index];
      
      unsigned long long mtree_leaf_size = mem_org -> ctrStoreSize / CACHE_LINE * MTREE_ENTRY_SIZE_VAR[0];

      if(mtree_leaf_size * 2 > mtree_total_size)
        mtree_size = mtree_leaf_size*2;
      else
        mtree_size = mtree_total_size;

      mem_org ->mtreeStoreSize = mtree_size;
      }
      MTREE_CTR_DESIGN = (int*) calloc(mem_org -> numMtreeLevels, sizeof(int));
      MTREE_ENTRY_SIZE =  (double*) calloc(mem_org -> numMtreeLevels, sizeof(double));
      MTREE_ARY =  (int*) calloc(mem_org -> numMtreeLevels, sizeof(int));

      for(int i=0; i<mem_org -> numMtreeLevels; i++){
        int index = i;
        if(index >=4){
          index = 4;
        }
        MTREE_CTR_DESIGN[mem_org->numMtreeLevels - 1 - i] = MTREE_CTR_DESIGN_VAR[index];
        MTREE_ENTRY_SIZE[mem_org->numMtreeLevels - 1 - i] = MTREE_ENTRY_SIZE_VAR[index]; 
        MTREE_ARY[mem_org->numMtreeLevels - 1 - i] = (int) (1.0*CACHE_LINE/MTREE_ENTRY_SIZE[mem_org->numMtreeLevels - 1 - i]);    
        // Leaf <-> numMtreeLevels -1 
      }

      
      //Initialize size of each level in bytes

      
      mem_org -> mtreeLevelSize = (unsigned long long*)  calloc(mem_org -> numMtreeLevels, sizeof(unsigned long long));
      mem_org -> mtreeLevelSize[0]= (CACHE_LINE); //Root is always single cacheline
      
      for(int i=1;i<mem_org->numMtreeLevels; i++){
        if(i==1){
          mem_org -> mtreeLevelSize[i] =  mem_org -> mtreeLevelSize[i-1] * mem_org -> numMtreeRoot; 
        }
        else{
          mem_org -> mtreeLevelSize[i] = mem_org -> mtreeLevelSize[i-1]*MTREE_ARY[i-1];      
        }
        //printf("Level %d size with arity %d: %llu B \n",i,MTREE_ARY[i],mem_org -> mtree_level_size[i]);
      }
      
      //ASSERTM( ((uns64)((mem_org -> mtree_level_size[mem_org->num_Mtree_levels-1]/MTREE_ENTRY_SIZE[mem_org->num_Mtree_levels-1])*(CTRS_PER_MTREE*CTR_SIZE)) ) == mem_org ->ctr_store_size,"Arithmetic regarding sizes of mtree & counters does not adds up\n");

      //Initialize mtree_levels_start_addr
      mem_org -> mtreeLevelsStartAddr = (Addr*) calloc(mem_org -> numMtreeLevels + 1, sizeof(Addr));
      // root
      mem_org -> mtreeLevelsStartAddr[0] = mem_org->memSize - mem_org->MACStoreSize - mem_org->ctrStoreSize - mtree_size; 
      // base or leaf
      mem_org -> mtreeLevelsStartAddr[mem_org -> numMtreeLevels] =  mem_org -> memSize - mem_org -> MACStoreSize - mem_org -> ctrStoreSize; 
      
      for(int i=1; i < mem_org -> numMtreeLevels; i++){
        if(i==1)
          // root is the size of cl
          mem_org -> mtreeLevelsStartAddr[i] = mem_org -> mtreeLevelsStartAddr[0] + CACHE_LINE;

        else{
          //mem_org -> mtree_levels_start_addr[i] =  mem_org -> mtree_levels_start_addr[i-1] + (uns64)((mem_org -> num_Mtree_root)*( ((ADDR)1)<<(twoexp_ary*(i-1)) )*MTREE_ENTRY_SIZE); 
          mem_org -> mtreeLevelsStartAddr[i] =  mem_org -> mtreeLevelsStartAddr[i-1] +  mem_org ->mtreeLevelSize[i-1];
        }
        //ASSERTM((mem_org -> mtree_levels_start_addr[i] - mem_org -> mtree_levels_start_addr[i-1] ) == mem_org ->mtree_level_size[i-1], "Address calculation tallies with mtree level sizes\n");
        
      }
    }

    //Return the MAC address, given a cacheline addr
    Addr getMACAddr( Addr cacheline_paddr, MemOrg * mem_org ){

      Addr start_MAC_paddr = mem_org -> memSize - mem_org -> MACStoreSize;
      Addr cacheline_num   = cacheline_paddr / CACHE_LINE; 


      Addr result_MAC_paddr = start_MAC_paddr + cacheline_num * MAC_SIZE ;

      return result_MAC_paddr;
      
    }


    //Return the Counter address, given a cacheline addr
    CtrMtreeEntry getCounterAddr( Addr cacheline_paddr, MemOrg * mem_org ){

      Addr start_Counter_paddr = mem_org -> memSize - mem_org -> MACStoreSize - mem_org -> ctrStoreSize;
      Addr cacheline_num   = cacheline_paddr / CACHE_LINE;

      Addr result_Counter_paddr = start_Counter_paddr + (Addr)(cacheline_num * CTR_SIZE);
      //Addr temp_result_Counter_paddr = start_Counter_paddr + (double)cacheline_num*(double)CTR_SIZE;
      
      Addr ctr_level_end_addr;

      CtrMtreeEntry dest;
      dest.mtreeLevel = mem_org -> numMtreeLevels; // base or Leaf
      dest.entryNum   = cacheline_num; // in the base or Leaf level, the entry number covers the whole memory region
      dest.paddr      = result_Counter_paddr;

      //ctr_level_end_addr = mem_org -> mtree_levels_start_addr[dest.mtree_level] + mem_org -> ctr_store_size;

      //ASSERTM(((dest.paddr >= start_Counter_paddr) && (dest.paddr < ctr_level_end_addr)),"Calculation mistake with getCounterAddr\n");	      

      return dest;

    }



    //Return  the Mtree address, given a counter/Mtree addr (go one level up in the tree)
    CtrMtreeEntry getMtreeEntry( CtrMtreeEntry src, MemOrg * mem_org ){

      //Assumed the size of Counter Store is 0.125*Mem_Size
      //Assumed the size of Mtree is 0.25 of the Counter Store
      //Leaf level of Mtree is 0.125*CTR_Store
      //Upper levels will be less than another 0.125*CTR_Store

      long long int num_counters = mem_org -> memSize / CACHE_LINE;
      long long int num_mtree_leaves = num_counters / CTRS_PER_MTREE;

      //  INT_64 Mtree_size = (mem_org -> ctr_store_size / MTREE_ARY * 2);
      //INT_64 Mtree_size =  (INT_64) (num_mtree_leaves * MTREE_ENTRY_SIZE * 2);  
      long long int Mtree_size = mem_org -> mtreeStoreSize;
      
      Addr Mtree_start = mem_org -> memSize - mem_org -> MACStoreSize - mem_org -> ctrStoreSize - Mtree_size; 

      //  ADDR Ctr_start = mem_org -> mem_size -  mem_org -> MAC_store_size - mem_org -> ctr_store_size;
        
      if(src.mtreeLevel == 0)
      //root
        return src;

      else if ( (src.mtreeLevel > 0) && (src.mtreeLevel <= mem_org -> numMtreeLevels ) ){
        Addr dest_level_end_addr   = 0 ;
        CtrMtreeEntry dest ;
        if(src.mtreeLevel == mem_org ->numMtreeLevels){
          //src is counter
          dest.entryNum = src.entryNum / CTRS_PER_MTREE;
        }
        else{
          //src is merkle tree
          dest.entryNum = src.entryNum / MTREE_ARY[src.mtreeLevel];
        } 
        //go one level higher
        dest.mtreeLevel = --src.mtreeLevel;


        dest.paddr = mem_org -> mtreeLevelsStartAddr[dest.mtreeLevel] 
                   + (unsigned long long) (dest.entryNum * MTREE_ENTRY_SIZE[dest.mtreeLevel]) ;

        //dest_level_end_addr = mem_org -> mtreeLevelsStartAddr[dest.mtreeLevel] + mem_org -> mtreeLevelSize[dest.mtreeLevel];
        // if(!((dest.paddr >= mem_org->mtree_levels_start_addr[dest.mtree_level]) && (dest.paddr < dest_level_end_addr))){
        //   printf("Dest Addr : %llu, dest entry: %llu, src addr: %llu, src entry :%llu, lower bound on mtree level :%llu, %llu, tight upper bound on mtree level :%llu\n",dest.paddr, dest.entry_num,src.entry_num, src.paddr, dest.mtree_level, mem_org->mtree_levels_start_addr[dest.mtree_level], dest_level_end_addr);
        // }    
        //ASSERTM( (dest.paddr >= mem_org->mtree_levels_start_addr[dest.mtree_level]) && (dest.paddr < dest_level_end_addr),"Calculation mistake with getMtreeEntry\n");	      

        //TODO:ADD assert about mtree_level of dest with paddr checl.
        //    std::cout << "Mtree_start " << Mtree_start << ", Mtree_level_start;
        // if(!((dest.paddr >= mem_org->mtree_levels_start_addr[dest.mtree_level]) && (dest.paddr < mem_org->mtree_levels_start_addr[dest.mtree_level+1]) && ((dest.mtree_level + 1) <= mem_org->num_Mtree_levels))){
        //   printf("Dest Addr : %llu, dest entry: %llu, src entry :%llu, lower bound on mtree level :%llu, %llu,  upper bound on mtree level :%llu, %llu \n",dest.paddr, dest.entry_num,src.entry_num, dest.mtree_level, mem_org->mtree_levels_start_addr[dest.mtree_level],dest.mtree_level+1,mem_org->mtree_levels_start_addr[dest.mtree_level+1]);
        // }
        
        //ASSERTM( (dest.paddr >= mem_org->mtree_levels_start_addr[dest.mtree_level]) && (dest.paddr < mem_org->mtree_levels_start_addr[dest.mtree_level+1]) && ((dest.mtree_level + 1) <= mem_org->num_Mtree_levels),"Calculation mistake with getMtreeEntry\n");



        return dest;

      }

      // else {
      //   printf( "Invalid invocation of getMtreeEntry\n");
      //   exit(1);
      // }
      
    }


    //Function to return size of usable memory in pages
    // This should only apply to home region
    // long long int getOSVisibleMem( memOrg * mem_org) {
    //   //INT_64 Mtree_size = (mem_org -> ctr_store_size / MTREE_ARY * 2);

    //   long long int num_counters = mem_org -> mem_size / CACHE_LINE;
    //   long long int num_mtree_leaves = num_counters / CTRS_PER_MTREE;
    //   //INT_64 Mtree_size =   num_mtree_leaves * MTREE_ENTRY_SIZE * 2;
    //   INT_64 Mtree_size =   mem_org->mtree_store_size;  
    //   INT_64 usable_memory = mem_org -> mem_size - mem_org -> MAC_store_size  -  mem_org -> ctr_store_size - Mtree_size;
      
    //   return (usable_memory); //Return memory in Bytes visible to OS
      
    // }

    //Function returns whether address belongs to data pages or not 
    int getPartition(Addr paddr, MemOrg * mem_org){
      //  INT_64 Mtree_size = (mem_org -> ctr_store_size / MTREE_ARY * 2);

      long long int num_counters = mem_org -> memSize / CACHE_LINE;
      long long int num_mtree_leaves = num_counters / CTRS_PER_MTREE;
      //INT_64 Mtree_size =   num_mtree_leaves * MTREE_ENTRY_SIZE * 2;
      long long int Mtree_size =   mem_org->mtreeStoreSize;  


      long long int usable_memory = mem_org -> memSize - mem_org -> MACStoreSize  -  mem_org -> ctrStoreSize - Mtree_size;
      if( (SGX_MODE[0] == 0) && (SGX_MODE[1] == 0) && (SGX_MODE[2] == 0) && (SGX_MODE[3] == 0) ){
        return 0;
      }

      if(paddr < usable_memory)
        return 0; //Usable memory
      else if( paddr < (usable_memory + Mtree_size) )
        return 1; //Merkle Tree Entries
      else if( paddr < (usable_memory + Mtree_size + mem_org -> ctrStoreSize ) )
        return 2; //Counters
      else if( paddr < (usable_memory + Mtree_size + mem_org -> ctrStoreSize + mem_org -> MACStoreSize ) )
        return 3; //MACs
      // else {
      // printf("********ERROR in get_partition in memOrg.c ********\n");
      // //printf("padddr: %llu, usable : %lld, mtree_store: %lld,  ctr_store; %lld , mac_store: %lld\n", paddr, usable_memory,Mtree_size, mem_org->ctr_store_size, mem_org->MAC_store_size);
      // exit(1);
      // }

    }

    // given a met_paddr, get its parent
    CtrMtreeEntry getMtreeEvictParent( Addr met_paddr, MemOrg * mem_org ){
      CtrMtreeEntry parent,child;
      int met_partition = getPartition(met_paddr, mem_org);
      
      // must be either ctr region or mtree region
      // ASSERTM( (met_partition == 1) || (met_partition == 2), "ERROR: getMtreEvictParent has been called with paddr not lying within MET" );

      int src_level = -1;
      int src_entry_num = -1;
      
      // get mtree level of the source cl
      for(int i= mem_org -> numMtreeLevels; i >= 0 ; i--){
      // from leaves to the root   max -> 0  
        if(met_paddr >= mem_org -> mtreeLevelsStartAddr[i]){
          src_level = i;
          break;
        }

      }
      //ASSERTM(src_level != -1, "ERROR: Unable to assert src_level in getMtreeEvictParent");


      if(src_level == mem_org-> numMtreeLevels){
        // if src_level is leaves (counters)
        src_entry_num = (unsigned long long) ( (met_paddr - mem_org -> mtreeLevelsStartAddr[src_level])/CTR_SIZE );
      }
      else{
        src_entry_num = (unsigned long long) ( (met_paddr - mem_org -> mtreeLevelsStartAddr[src_level])/MTREE_ENTRY_SIZE[src_level] );
      }
      
      child.paddr       = met_paddr;
      child.mtreeLevel  = src_level;
      child.entryNum    = src_entry_num;

      //  printf("Input Child addr: %llu, level: %d, entry_num: %llu\n",child.paddr,child.mtree_level,child.entry_num);
      parent = getMtreeEntry(child, mem_org);
      return parent;
    }


    Addr getCtrChild( Addr met_paddr, MemOrg * mem_org, int *num_children ){
      CtrMtreeEntry parent;
      Addr child_paddr = 0, child_level_startaddr = 0;
      int met_partition = getPartition(met_paddr, mem_org);

      //ASSERTM( (met_partition == 1) || (met_partition == 2), "ERROR: getMtreEvictParent has been called with paddr not lying within MET" );

      // Ensure met_paddr is aligned to cacheline
      // 2^^6=64
      met_paddr = met_paddr >> 6;
      met_paddr = met_paddr << 6;
      
      int src_level = -1;
      unsigned long long src_entry_num = -1;
      
      for(int i= mem_org -> numMtreeLevels; i >= 0 ; i--){

        if(met_paddr >= mem_org -> mtreeLevelsStartAddr[i]){
          src_level = i;
          break;
        }

      }
      //ASSERTM(src_level != -1, "ERROR: Unable to assert src_level in getMtreeEvictParent");

      if(src_level == mem_org-> numMtreeLevels){
        // leaf
        src_entry_num = (met_paddr - mem_org -> mtreeLevelsStartAddr[src_level])/CTR_SIZE;
        //ASSERTM( ((uns64)(src_entry_num*CTR_SIZE)) % CACHE_LINE_SIZE == 0, "src_entry_num should be first in cacheline\n");
      }
      else{
        src_entry_num = (met_paddr - mem_org -> mtreeLevelsStartAddr[src_level])/MTREE_ENTRY_SIZE[src_level];
          
        //ASSERTM( ((uns64)(src_entry_num * MTREE_ENTRY_SIZE[src_level])) % CACHE_LINE_SIZE == 0, "src_entry_num should be first in cacheline\n");
      }

            
      if(src_level == mem_org-> numMtreeLevels){
        child_level_startaddr = 0; //DATA start addr
        *num_children = (unsigned long long)( CACHE_LINE_SIZE / CTR_SIZE);
      }
      else {
        child_level_startaddr  = mem_org -> mtreeLevelsStartAddr[src_level + 1];
        *num_children = (unsigned long long) (CACHE_LINE_SIZE / MTREE_ENTRY_SIZE[src_level]);
      }

      child_paddr = child_level_startaddr + src_entry_num * CACHE_LINE_SIZE;
      return child_paddr;

    }
  }
}



