# Cache Simulator

Project Implements a flexible cache and memory hierarchy simulator and uses it to study the performance of memory hierarchies using the SPEC benchmarks. 

## Memory Hierarchy

Simulator is capable of implementing 2 level caches with option of L2 being a Decoupled sector cache. Simulator reads trace files and assigns request to L1 cache. L1 cache send read/write requests to L2 cache. L2 cache interacts with memory. L1 and L2 cache keep track of their own counter i.e. reads, writes, miss, hits etc. At end of simulation program prints stats for both caches.

## Configurable Parameters
* BLOCKSIZE: Number of bytes in a block.
* L1_Size: Total bytes of data storage for L1 Cache.
* L1_Assoc: Associativity of L1 Cache.
* L2_Size: Total bytes of data storage for L2 Cache.
* L2_Assoc: Associativity of L2 Cache.
* L2_Data_Blocks: Number of data blocks in a seector.
* L2_Addr_Tags: Number of address tags pointing to a sector.
* Trace_file: Input Trace file

## Replacement policy
Simulator uses least replacement policy(LRU)

## Write policy
Simulator uses Write-back Write-allocate(WBWA) policy

## Running the simulator
**$./sim_cache \<BLOCKSIZE> <L1_Size> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <L2_Data_Blocks> <L2_Addr_Tags> <Trace_file>**

e.g.
$./sim_cache 64 1024 2 65536 8 8 2 gcc_trace.txt

