#include <stdio.h>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdlib.h>
#include <iomanip>
#include "CACHE.h"
#include "constants.h"

using namespace std;

unsigned long L1_BLOCKSIZE;       //Blocksize of L1 Cache.
unsigned long L1_SIZE;            //Total bytes of data storage for L1 Cache.
unsigned long L1_ASSOC;           //Associativity of L1 cache (ASSOC=1 is a direct-mapped cache)
unsigned long L1_DATA_BLOCKS = 1; //Number of data blocks in a sector.
unsigned long L1_ADDR_TAGS = 1;   //Number of address tags pointing to a sector.
unsigned long L2_BLOCKSIZE;       //Blocksize of L2 Cache.
unsigned long L2_SIZE;            //Total bytes of data storage for L2 Cache.
unsigned long L2_ASSOC;           //Associativity of L2 cache.
unsigned long L2_DATA_BLOCKS;     //Number of data blocks in a sector.
unsigned long L2_ADDR_TAGS;       //Number of address tags pointing to a sector.
char *Trace_file;                 //Input Trace file

int main(int argc, char *argv[])
{
    L1_BLOCKSIZE = strtoul(argv[1], 0, 10);
    L2_BLOCKSIZE = L1_BLOCKSIZE;
    L1_SIZE = strtoul(argv[2], 0, 10);
    L1_ASSOC = strtoul(argv[3], 0, 10);
    L2_SIZE = strtoul(argv[4], 0, 10);
    L2_ASSOC = strtoul(argv[5], 0, 10);
    L2_DATA_BLOCKS = strtoul(argv[6], 0, 10);
    L2_ADDR_TAGS = strtoul(argv[7], 0, 10);
    Trace_file = argv[8];

    cout << "  ===== Simulator configuration =====";
    cout << "\n  BLOCKSIZE:\t\t" << L1_BLOCKSIZE;
    cout << "\n  L1_SIZE:\t\t" << L1_SIZE;
    cout << "\n  L1_ASSOC:\t\t" << L1_ASSOC;
    cout << "\n  L2_SIZE:\t\t" << L2_SIZE;
    cout << "\n  L2_ASSOC:\t\t" << L2_ASSOC;
    cout << "\n  L2_DATA_BLOCKS:\t" << L2_DATA_BLOCKS;
    cout << "\n  L2_ADDRESS_TAGS:\t" << L2_ADDR_TAGS;
    cout << "\n  trace_file:\t\t" << Trace_file << "\n";

    CACHE L2;
    CACHE L1;
    if (L2_SIZE != 0)
    {
        //Initialize L2 CACHE
        L2 = CACHE(L2_BLOCKSIZE, L2_SIZE, L2_ASSOC, L2_DATA_BLOCKS, L2_ADDR_TAGS, NULL);

        //Initialize L1 CACHE
        L1 = CACHE(L1_BLOCKSIZE, L1_SIZE, L1_ASSOC, L1_DATA_BLOCKS, L1_ADDR_TAGS, &L2);
    }
    else
    {
        //Initialize L1 CACHE
        L1 = CACHE(L1_BLOCKSIZE, L1_SIZE, L1_ASSOC, L1_DATA_BLOCKS, L1_ADDR_TAGS, NULL);
    }
    //Read trace file
    FILE *fp;
    fp = fopen(Trace_file, "r");
    if (fp == NULL)
    {
        //If file cannot be read exit
        exit(0);
    }

    char operation;                   //Read or write operation.
    char addressString[ADDRESS_SIZE]; //Address to be accessed in string form
    unsigned long address;            //Address to be accessed in integer form
    char temp[1];

    stringstream ss;
    operation = fgetc(fp);
    while (operation != EOF)
    {
        fgets(temp, sizeof(char), (FILE *)fp);          //To skip the space in between
        fgets(addressString, ADDRESS_SIZE, (FILE *)fp); //To read the address string

        //Conversion of Hex string to number
        ss << hex << addressString;
        ss >> address;

        if (operation == 'r' || operation == 'R')
        {
            L1.readFromAddress(address);
        }
        else if (operation == 'w' || operation == 'W')
        {
            char data = 0;
            L1.writeToAddress(address);
        }

        operation = fgetc(fp);
    }
    fclose(fp);

    cout << "\n===== L1 contents =====";
    L1.CacheStatus();

    if (L2_SIZE != 0)
    {
        if (L2_ADDR_TAGS == 1 && L2_DATA_BLOCKS == 1)
        {
            cout << "\n\n===== L2 contents =====";
        }
        L2.CacheStatus();
    }

    cout << "\n\n===== Simulation Results =====";
    cout << "\na. number of L1 reads:\t\t\t" << dec << L1.READ;
    cout << "\nb. number of L1 read misses:\t\t" << dec << L1.READ_MISS;
    cout << "\nc. number of L1 writes:\t\t\t" << dec << L1.WRITE;
    cout << "\nd. number of L1 write misses:\t\t" << dec << L1.WRITE_MISS;
    cout << "\ne. L1 miss rate:\t\t\t" << fixed << setprecision(4) << L1.MISS_RATE;
    cout << "\nf. number of writebacks from L1 memory:\t" << dec << L1.WRITE_BACKS;
    if (L2_SIZE != 0)
    {
        cout << "\ng. number of L2 reads:\t\t\t" << dec << L2.READ;
        cout << "\nh. number of L2 read misses:\t\t" << dec << L2.READ_MISS;
        cout << "\ni. number of L2 writes:\t\t\t" << dec << L2.WRITE;
        cout << "\nj. number of L2 write misses:\t\t" << dec << L2.WRITE_MISS;

        if (L2_ADDR_TAGS == 1 && L2_DATA_BLOCKS == 1)
        {
            cout << "\nk. L2 miss rate:\t\t\t" << fixed << setprecision(4) << (float)L2.READ_MISS / (L2.READ);
            cout << "\nl. number of writebacks from L2 memory:\t" << dec << L2.WRITE_BACKS;
            cout << "\nm. total memory traffic:\t\t" << dec << L2.WRITE_MISS + L2.READ_MISS + L2.WRITE_BACKS << "\n";
        }
        else
        {
            cout << "\nk. number of L2 sector misses:\t\t" << L2.SECTOR_MISS;
            cout << "\nl. number of L2 cache block misses:\t" << dec << L2.CACHE_BLOCK_MISS;
            cout << "\nm. L2 miss rate:\t\t\t" << fixed << setprecision(4) << (float)L2.READ_MISS / (L2.READ);
            cout << "\nn. number of writebacks from L2 memory:\t" << dec << L2.WRITE_BACKS;
            cout << "\no. total memory traffic:\t\t" << dec << L2.WRITE_MISS + L2.READ_MISS + L2.WRITE_BACKS << "\n";
        }
    }
    else
    {
        cout << "\ng. total memory traffic:\t\t" << dec << L1.WRITE_MISS + L1.READ_MISS + L1.WRITE_BACKS << "\n";
    }

    return 0;
}