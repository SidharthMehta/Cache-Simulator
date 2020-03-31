#include "CACHE.h"
#include <math.h>
#include "constants.h"
#include <iostream>

using namespace std;

CACHE::CACHE()
{
}

CACHE::CACHE(unsigned long BLOCKSIZE, unsigned long CACHE_SIZE, unsigned long CACHE_ASSOC, unsigned long DATA_BLOCKS, unsigned long ADDR_TAGS, CACHE *nextLevel)
{
    this->BLOCKSIZE = BLOCKSIZE;
    this->CACHE_SIZE = CACHE_SIZE;
    this->CACHE_ASSOC = CACHE_ASSOC;
    this->DATA_BLOCKS = DATA_BLOCKS;
    this->ADDR_TAGS = ADDR_TAGS;
    this->nextLevel = nextLevel;

    SETS = CACHE_SIZE / (BLOCKSIZE * CACHE_ASSOC * DATA_BLOCKS);

    //Setting width of address bits
    BLOCK_OFFSET_BITS = ceil(log2(BLOCKSIZE));
    DATA_BLOCK_BITS = ceil(log2(DATA_BLOCKS));
    INDEX_BITS = ceil(log2(SETS));
    ADDR_TAG_BITS = ceil(log2(ADDR_TAGS));
    TAG_BITS = ADDRESS_SIZE - ADDR_TAG_BITS - INDEX_BITS - DATA_BLOCK_BITS - BLOCK_OFFSET_BITS;

    //Calculation of masks
    for (unsigned long i = 0; i < BLOCK_OFFSET_BITS; i++)
    {
        BLOCK_OFFSET_MASK = (BLOCK_OFFSET_MASK << 1) + 1;
    }

    for (unsigned long i = 0; i < DATA_BLOCK_BITS; i++)
    {
        DATA_BLOCK_MASk = (DATA_BLOCK_MASk << 1) + 1;
    }
    DATA_BLOCK_MASk = DATA_BLOCK_MASk << BLOCK_OFFSET_BITS;

    for (unsigned long i = 0; i < INDEX_BITS; i++)
    {
        INDEX_MASK = (INDEX_MASK << 1) + 1;
    }
    INDEX_MASK = INDEX_MASK << (DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);

    for (unsigned long i = 0; i < ADDR_TAG_BITS; i++)
    {
        ADDR_TAG_MASK = (ADDR_TAG_MASK << 1) + 1;
    }
    ADDR_TAG_MASK = ADDR_TAG_MASK << (INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);

    for (unsigned long i = 0; i < TAG_BITS; i++)
    {
        TAG_MASK = (TAG_MASK << 1) + 1;
    }
    TAG_MASK = TAG_MASK << (ADDR_TAG_BITS + INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);

    //Initializing the structure
    tagStore = new TAGSTORE *[SETS];
    for (unsigned long i = 0; i < SETS; i++)
    {
        tagStore[i] = new TAGSTORE[CACHE_ASSOC];
    }

    for (unsigned long i = 0; i < SETS; i++)
    {
        for (unsigned long j = 0; j < CACHE_ASSOC; j++)
        {
            tagStore[i][j].TAG = new long long[ADDR_TAGS];
            for (unsigned long k = 0; k < ADDR_TAGS; k++)
            {
                tagStore[i][j].TAG[k] = vacant;
            }
            tagStore[i][j].frequency = j;

            tagStore[i][j].dataStore.TagSelect = new unsigned long[DATA_BLOCKS];
            tagStore[i][j].dataStore.isDirty = new bool[DATA_BLOCKS];
            tagStore[i][j].dataStore.isValid = new bool[DATA_BLOCKS];
            for (unsigned long k = 0; k < DATA_BLOCKS; k++)
            {
                tagStore[i][j].dataStore.TagSelect[k] = 0;
                tagStore[i][j].dataStore.isDirty[k] = false;
                tagStore[i][j].dataStore.isValid[k] = false;
            }
        }
    }
}

void CACHE::CacheStatus()
{
    if (ADDR_TAGS > 1 || DATA_BLOCKS > 1)
    {
        cout << "\n\n===== L2 Address Array contents =====";
    }
    for (unsigned long i = 0; i < SETS; i++)
    {
        cout << "\nset\t" << dec << i << ":\t";
        for (unsigned long j = 0; j < CACHE_ASSOC; j++)
        {
            for (unsigned long k = 0; k < CACHE_ASSOC; k++)
            {
                if (tagStore[i][k].frequency == j)
                {
                    if (ADDR_TAGS > 1 || DATA_BLOCKS > 1)
                    {
                        for (unsigned long l = 0; l < ADDR_TAGS; l++)
                        {
                            if (tagStore[i][k].TAG[l] == vacant)
                            {
                                cout << "\t0";
                            }
                            else
                            {
                                cout << "\t" << hex << tagStore[i][k].TAG[l];
                            }
                        }
                        cout << "\t||";
                    }
                    else if (DATA_BLOCKS == 1)
                    {

                        if (tagStore[i][k].TAG[0] == vacant)
                        {
                            cout << "\t0 ";
                        }
                        else
                        {
                            cout << hex << tagStore[i][k].TAG[0] << " ";
                        }

                        if (tagStore[i][k].dataStore.isDirty[0] == true)
                        {
                            cout << "D ||\t";
                        }
                        else
                        {
                            cout << "N ||\t";
                        }
                    }
                }
            }
        }
    }
    if (ADDR_TAGS > 1 || DATA_BLOCKS > 1)
    {
        cout << "\n\n===== L2 Data Array contents =====";
        for (unsigned long i = 0; i < SETS; i++)
        {
            cout << "\nset\t" << dec << i << ":\t";
            for (unsigned long j = 0; j < CACHE_ASSOC; j++)
            {
                for (unsigned long k = 0; k < CACHE_ASSOC; k++)
                {
                    if (tagStore[i][k].frequency == j)
                    {
                        for (unsigned long l = 0; l < DATA_BLOCKS; l++)
                        {
                            cout << "\t" << hex << tagStore[i][k].dataStore.TagSelect[l] << ",";
                            if (tagStore[i][k].dataStore.isValid[l])
                            {
                                cout << "V,";
                            }
                            else
                            {
                                cout << "I,";
                            }

                            if (tagStore[i][k].dataStore.isDirty[l])
                            {
                                cout << "D";
                            }
                            else
                            {
                                cout << "N";
                            }
                        }
                        cout << "\t||";
                    }
                }
            }
        }
    }

    MISS_RATE = ((float)(READ_MISS + WRITE_MISS)) / (READ + WRITE);
}

void CACHE::readFromAddress(unsigned long address)
{
    READ++;
    //Calculation of bits
    unsigned long TAG = (address & TAG_MASK) >> (ADDRESS_SIZE - TAG_BITS);
    unsigned long addrTAG = (address & ADDR_TAG_MASK) >> (INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);
    unsigned long index = (address & INDEX_MASK) >> (DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);
    unsigned long sector = (address & DATA_BLOCK_MASk) >> (BLOCK_OFFSET_BITS);
    //unsigned long blockOffset = address & BLOCK_OFFSET_MASK;

    //Iterate through data blocks of a set(index) and check for tag match and valid sector
    for (unsigned long j = 0; j < CACHE_ASSOC; j++)
    {
        if (tagStore[index][j].TAG[addrTAG] == TAG && tagStore[index][j].dataStore.TagSelect[sector] == addrTAG && tagStore[index][j].dataStore.isValid[sector] == true)
        {
            READ_HIT++;
            resetFrequency(index, tagStore[index][j].frequency);
            return;
        }
    }

    READ_MISS++;

    //for (long k = CACHE_ASSOC - 1; k >= 0; k--)
    {
        //Iterate through data blocks of a set(index) and check for tag match and invalid cache block
        for (unsigned long j = 0; j < CACHE_ASSOC; j++)
        {
            //if (tagStore[index][j].frequency == k)
            {
                if (tagStore[index][j].TAG[addrTAG] == TAG && tagStore[index][j].dataStore.isValid[sector] == false)
                {
                    checkMissType(index, j);
                    requestReadFromNext(address);
                    //Write data in empty cache block and make it valid and set its tagSelect
                    tagStore[index][j].dataStore.isValid[sector] = true;
                    tagStore[index][j].dataStore.TagSelect[sector] = addrTAG;
                    resetFrequency(index, tagStore[index][j].frequency);
                    return;
                }
            }
        }
    }

    //for (long k = CACHE_ASSOC - 1; k >= 0; k--)
    {
        //If data block is not empty find empty with first way
        for (unsigned long j = 0; j < CACHE_ASSOC; j++)
        {
            //if (tagStore[index][j].frequency == k)
            {
                //Check if tag at ADDR BIT is empty and there is no data at that cache block
                if (tagStore[index][j].TAG[addrTAG] == vacant && tagStore[index][j].dataStore.isValid[sector] == false)
                {
                    checkMissType(index, j);
                    requestReadFromNext(address);
                    //Write data in empty cache block and make it valid and set its tagSelect
                    tagStore[index][j].TAG[addrTAG] = TAG;
                    tagStore[index][j].dataStore.isValid[sector] = true;
                    tagStore[index][j].dataStore.TagSelect[sector] = addrTAG;
                    resetFrequency(index, tagStore[index][j].frequency);
                    return;
                }
            }
        }
    }
    unsigned long assocReplacement = evict(address);
    requestReadFromNext(address);
    tagStore[index][assocReplacement].TAG[addrTAG] = TAG;
    tagStore[index][assocReplacement].dataStore.TagSelect[sector] = addrTAG;
    tagStore[index][assocReplacement].dataStore.isValid[sector] = true;
    tagStore[index][assocReplacement].dataStore.isDirty[sector] = false;
    resetFrequency(index, tagStore[index][assocReplacement].frequency);
}

void CACHE::writeToAddress(unsigned long address)
{
    WRITE++;
    //Calculation of bits
    unsigned long TAG = (address & TAG_MASK) >> (ADDRESS_SIZE - TAG_BITS);
    unsigned long addrTAG = (address & ADDR_TAG_MASK) >> (INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);
    unsigned long index = (address & INDEX_MASK) >> (DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);
    unsigned long sector = (address & DATA_BLOCK_MASk) >> (BLOCK_OFFSET_BITS);
    //unsigned long blockOffset = address & BLOCK_OFFSET_MASK;

    //Iterate through data blocks of a set(index) and check for tag match and valid sector
    for (unsigned long j = 0; j < CACHE_ASSOC; j++)
    {
        if (tagStore[index][j].TAG[addrTAG] == TAG && tagStore[index][j].dataStore.TagSelect[sector] == addrTAG && tagStore[index][j].dataStore.isValid[sector] == true)
        {
            WRITE_HIT++;
            tagStore[index][j].dataStore.isDirty[sector] = true;
            resetFrequency(index, tagStore[index][j].frequency);
            return;
        }
    }

    WRITE_MISS++;

    //for (long k = CACHE_ASSOC - 1; k >= 0; k--)
    {
        //Iterate through data blocks of a set(index) and check for tag match and invalid cache block
        for (unsigned long j = 0; j < CACHE_ASSOC; j++)
        {
            //if (tagStore[index][j].frequency == k)
            {
                if (tagStore[index][j].TAG[addrTAG] == TAG && tagStore[index][j].dataStore.isValid[sector] == false)
                {
                    checkMissType(index, j);
                    requestReadFromNext(address);
                    //Write data in empty cache block and make it valid and set its tagSelect
                    tagStore[index][j].dataStore.isValid[sector] = true;
                    tagStore[index][j].dataStore.isDirty[sector] = true;
                    tagStore[index][j].dataStore.TagSelect[sector] = addrTAG;
                    resetFrequency(index, tagStore[index][j].frequency);
                    return;
                }
            }
        }
    }

    //for (long k = CACHE_ASSOC - 1; k >= 0; k--)
    {
        //If data block is not empty find empty with first way
        for (unsigned long j = 0; j < CACHE_ASSOC; j++)
        {
            //if (tagStore[index][j].frequency == k)
            {
                //Check if tag at ADDR BIT is empty and there is no data at that cache block
                if (tagStore[index][j].TAG[addrTAG] == vacant && tagStore[index][j].dataStore.isValid[sector] == false)
                {
                    checkMissType(index, j);
                    requestReadFromNext(address);
                    //Write data in empty cache block and make it valid and set its tagSelect
                    tagStore[index][j].TAG[addrTAG] = TAG;
                    tagStore[index][j].dataStore.isValid[sector] = true;
                    tagStore[index][j].dataStore.isDirty[sector] = true;
                    tagStore[index][j].dataStore.TagSelect[sector] = addrTAG;
                    resetFrequency(index, tagStore[index][j].frequency);
                    return;
                }
            }
        }
    }
    unsigned long assocReplacement = evict(address);
    requestReadFromNext(address);
    tagStore[index][assocReplacement].TAG[addrTAG] = TAG;
    tagStore[index][assocReplacement].dataStore.TagSelect[sector] = addrTAG;
    tagStore[index][assocReplacement].dataStore.isValid[sector] = true;
    tagStore[index][assocReplacement].dataStore.isDirty[sector] = true;
    resetFrequency(index, tagStore[index][assocReplacement].frequency);
}
unsigned long CACHE::evict(unsigned long address)
{
    //Calculation of bits
    unsigned long TAG = (address & TAG_MASK) >> (ADDRESS_SIZE - TAG_BITS);
    unsigned long addrTAG = (address & ADDR_TAG_MASK) >> (INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);
    unsigned long index = (address & INDEX_MASK) >> (DATA_BLOCK_BITS + BLOCK_OFFSET_BITS);
    unsigned long sector = (address & DATA_BLOCK_MASk) >> (BLOCK_OFFSET_BITS);
    //unsigned long blockOffset = address & BLOCK_OFFSET_MASK;

    //Finding dataBlock where LRU is minimum
    unsigned long assocReplacement;
    for (unsigned long j = 0; j < CACHE_ASSOC; j++)
    {
        if (tagStore[index][j].frequency == (CACHE_ASSOC - 1))
        {
            assocReplacement = j;
            break;
        }
    }

    //If match case then clear only cache block
    if (tagStore[index][assocReplacement].TAG[addrTAG] == TAG)
    {
        checkMissType(index, assocReplacement);
        //Evict data only at cache block level
        if (tagStore[index][assocReplacement].dataStore.isDirty[sector] == true && tagStore[index][assocReplacement].dataStore.isValid[sector] == true)
        {
            unsigned long evictTag = tagStore[index][assocReplacement].dataStore.TagSelect[sector];
            unsigned long oldAddress = (tagStore[index][assocReplacement].TAG[evictTag] << (ADDR_TAG_BITS + INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS)) +
                                       (evictTag << (INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS)) + (index << (DATA_BLOCK_BITS + BLOCK_OFFSET_BITS)) + (sector << BLOCK_OFFSET_BITS);
            requestWriteToNext(oldAddress);
        }
        tagStore[index][assocReplacement].dataStore.isValid[sector] = false;
        tagStore[index][assocReplacement].dataStore.isDirty[sector] = false;
    }
    else
    {
        checkMissType(index, assocReplacement);
        //loop to iterate over cache blocks in a sector
        for (unsigned long k = 0; k < DATA_BLOCKS; k++)
        {
            //Write Back everything which is dirty for evicted tag having same select bit or evict data only at cache block level
            if (tagStore[index][assocReplacement].dataStore.isValid[k] &&
                (tagStore[index][assocReplacement].dataStore.TagSelect[k] == addrTAG || k == sector ))
            {
                if (tagStore[index][assocReplacement].dataStore.isDirty[k])
                {
                    unsigned long evictTag = tagStore[index][assocReplacement].dataStore.TagSelect[k];
                    unsigned long oldAddress = (tagStore[index][assocReplacement].TAG[evictTag] << (ADDR_TAG_BITS + INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS)) +
                                               (evictTag << (INDEX_BITS + DATA_BLOCK_BITS + BLOCK_OFFSET_BITS)) + (index << (DATA_BLOCK_BITS + BLOCK_OFFSET_BITS)) + (k << BLOCK_OFFSET_BITS);
                    requestWriteToNext(oldAddress);
                }
                tagStore[index][assocReplacement].dataStore.isValid[k] = false;
                tagStore[index][assocReplacement].dataStore.isDirty[k] = false;
                tagStore[index][assocReplacement].dataStore.TagSelect[k] = 0;
            }
        }
    }

    return assocReplacement;
}

void CACHE::resetFrequency(unsigned long index, unsigned long frequency)
{
    for (unsigned long j = 0; j < CACHE_ASSOC; j++)
    {
        if (tagStore[index][j].frequency < frequency)
        {
            tagStore[index][j].frequency++;
        }
        else if (tagStore[index][j].frequency == frequency)
        {
            tagStore[index][j].frequency = 0;
        }
    }
}

void CACHE::requestWriteToNext(unsigned long address)
{
    WRITE_BACKS++;
    if (nextLevel != NULL)
    {
        nextLevel->writeToAddress(address);
    }
}

void CACHE::requestReadFromNext(unsigned long address)
{
    if (nextLevel != NULL)
    {
        nextLevel->readFromAddress(address);
    }
}

void CACHE::checkMissType(unsigned long index, unsigned long assoc)
{
    bool isBlockMiss = false;
    bool isSectorMiss = true;
    for (unsigned long k = 0; k < DATA_BLOCKS; k++)
    {
        if (tagStore[index][assoc].dataStore.isValid[k])
        {
            isBlockMiss = true;
            isSectorMiss = false;
            break;
        }
    }
    if(isBlockMiss)
    {
        CACHE_BLOCK_MISS++;
    }
    else if(isSectorMiss)
    {
        SECTOR_MISS++;
    }    
}