#ifndef BLOOM_H
#define BLOOM_H

using namespace std;
//Nodes for singly linked list in auxilary hash table
struct node {
    string val;
    struct node* next;
  };



//Class for auxilary hash table
//table uses separate chaining and has linked lists in each bucket
class HashTable {
  public:
    HashTable(int q);  //constructor
    ~HashTable();      //destructor
    void insert (string element);  //insert into hash table
    void remove (string element); //remove value from hash table
    bool find (string element); //find in hash table
    int hash(string element); //makes an index to a bucket in the hash table from a given string 
    int m; //size of the hash table
    node** hashTable; // array of node pointers for the hashtable and linked lists.
    void print(); //printing method for testing
  private: 
    
};

class BloomFilter{
    public:
    //normal constructor.
    //p = probability of false positive
    //m = expected number of elements which will be added to the bloom filter
    //c = scale factor of bloom filter size
    //d = scale factor of number of hash functions

    BloomFilter(double p, int m, float c, float d); 
        ~BloomFilter(); //destructor for Bloom Filter
        void insert(string element); //insert into the Bloom Filter
        void remove(string element); //Remove from the Bloom Filter by adding to the Hash Table
        bool find(string element); //Check if a string exists in the Bloom Filter
        int BloomFilterSize(double p, int m, float c); //Calculates the size the Bloom Filter using the equation given in class
        int numHashFunctions(int n, int m, float d); //Calculates the number of hash functions using the equation from class
       //Converts a number into a index in the bloom filter
       //element is an int which should be the int associated with a string which will be inputed into the bloom filter
       //index is an int which represents which hash function will be chosen from a family of functions to use on element.
        int hash(unsigned int element, int index); 
        void print();  //Print out bloom filter for testing purposes

        //Data
        unsigned int numElem; //expected number of elements added into the bloom filter
        unsigned int size; //size of the bloom filter
        unsigned long long k; //First prime number greater than size. Used in the hashing function
        unsigned int pr; //expected probability of false positive
        int q; //size of the remove hash table
        vector<pair<unsigned int, unsigned int> > hFunc; //vector carrying pairs of random ints used for the family of hashing functions
        bool* bt; //array of booleans for the bloom filter
        HashTable* ht; //remove hash table


};


unsigned int strToInt(string element);






















#endif
