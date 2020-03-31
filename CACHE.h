struct DATASTORE
{
    unsigned long* TagSelect;   //selection bit array
    bool *isValid;
    bool *isDirty;
};

struct TAGSTORE
{
    long long* TAG;         //TAG array of size ADDR_TAGS
    unsigned long frequency;
    DATASTORE dataStore;
};

class CACHE{

    private:
        unsigned long BLOCKSIZE;   //Number of bytes in a block.
        unsigned long CACHE_SIZE;  //Size of Cache.
        unsigned long CACHE_ASSOC; //Associativity of cache.
        unsigned long DATA_BLOCKS;      //No of sectors
        unsigned long ADDR_TAGS;        //Tags per Set
        struct TAGSTORE **tagStore;    //tag structure
        CACHE *nextLevel;              //Next level of memory hierarchy

        //Width of address fields
        unsigned long SETS;
        unsigned long BLOCK_OFFSET_BITS;
        unsigned long INDEX_BITS;
        unsigned long TAG_BITS;
        unsigned long ADDR_TAG_BITS;
        unsigned long DATA_BLOCK_BITS;

        //Masks
        unsigned long TAG_MASK = 0;
        unsigned long INDEX_MASK = 0;
        unsigned long BLOCK_OFFSET_MASK = 0;
        unsigned long ADDR_TAG_MASK = 0;
        unsigned long DATA_BLOCK_MASk = 0;

        //Least recently used replacement policy
        unsigned long evict(unsigned long);

        //Method to update frequency of access
        void resetFrequency(unsigned long, unsigned long);

        //Write to next level of Memory Hierarchy
        void requestWriteToNext(unsigned long); 

        //Read from next level of Memory Hierarchy
        void requestReadFromNext(unsigned long);

        void checkMissType(unsigned long, unsigned long);

    public:

        //Performance parameters;
        unsigned long READ = 0;
        unsigned long READ_MISS = 0;
        unsigned long READ_HIT = 0;
        unsigned long WRITE = 0;
        unsigned long WRITE_MISS = 0;
        unsigned long WRITE_HIT = 0;
        unsigned long WRITE_BACKS = 0;
        unsigned long SECTOR_MISS = 0;
        unsigned long CACHE_BLOCK_MISS = 0;
        float MISS_RATE = 0;

        //Default
        CACHE();

        //Constructor
        CACHE(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, CACHE*);
        
        //Display contents of CACHE
        void CacheStatus();

        //Read from CACHE
        void readFromAddress(unsigned long);

        //Write to CACHE
        void writeToAddress(unsigned long);
};