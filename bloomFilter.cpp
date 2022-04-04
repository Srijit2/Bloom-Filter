#include <iostream>
#include <vector>
#include <string>
#include "bloomFilter.h"
#include <math.h> 
#include <stdlib.h>     
#include <time.h>  
#include <utility> 
#include<fstream>    


//Document write up:
//String to int conversion:
//For this I used the equation given in the instructions: x = d[0] * C^0 + d[1]*C^1 + ... + d[n]C^n.
//For this I picked the value for C to be 53 because it the prime number which is roughly equal to the
//size of unique letters(upper and lower).

//I accounted for overflow by modding the sum by the max unsigned int number after each summation,
//making the summation variable a long long, so no addition/multiplication of two ints could cause 
//overflow, and by modding my C variable by the max unsigned int variable to ensure that c was never 
//above 32 bits when it was used in the summation. C was also made as a long long for when it reached
//above 32 bits prior to it being modded back down.

//This prevents collisions between strings with the same letters in differing orders because the 
//constant C is constantly being multiplied by itself each summation. Which means that not only 
//the characters in a string matter, but also the order in which the appear.
//In the case of 'reed' vs 'deer'
//'reed' = 114*53^0 + 101*53^1 + 101*53^2 + 100^53^3 = 15176876
//'deer' = 100*53^0 + 101*53^1 + 101*53^2 + 114^53^3 = 17261140
//This shows that 'reed' and 'deer' wont collide unless the size of the bloom filter is the size of a 
//factor which they both have. 

//Bloom Filter Hashing:
//The hash function I chose to model my family after is ((ax+b) mod p) mod m) (from the textbook p. 231)
//Here a and b are randomly generated values where 1 <= a <= p-1 and 1 <= b <= p-1
//x is the int equivalent of the string which needs to be added to the filter. (calculations for this shown above)
//m is the size of the bloom filter 
//p is the first prime number bigger than m
//The point of a and b in the formula is to add extra variability to the function, so that it can
//be used in a family. 
//p is specifically a prime number, bigger than m, so that it will not have any common factors with
//(ax + b) which can cause bias in hashing.
//The mod m at the end is so that the value will be an index in the bloom filter.
//This equation was utilized to make a family by changing the values of a and b to allow for different 
//hashing functions. For example a function with a = 33490 and b = 35 would give completely different
//values than a = 345 and b = 12345. And because the equation is universal, all equations derived
//from the formula will also be universal.
//To make the family I created an array of random pairs for a and b, each of which were used as their
//own hash function.

//Remove Hash Table Size:
//I am assuming that roughly 10 percent of the input strings will be deleted. Based on that assumption
//I made the Hash Table size the next highest prime number following 1/10th of the expected bloom filter
//entries.

//Results:
//The pictures of the plots for testing are in the folder.
//It appears that the amount of false positive drops as the bloom filter scalar increases, which makes
//sense as there are more spots for each hash function to choose from, so the probabilty of two different
//string being matched to the same indices are lower.
//When the number of hash functions scalar increases, it first makes a positve effect dropping the 
//probabilty of a false positve, but after a certain point it increases the probabilty significantly.
//It initally drops it because two strings have to match each other for more functions which decreases
//the probabilty of collisions. However, if the number of hash functions get too high, then after a while
//the entire table will become full, which means that no matter what a string is, it will always think
//it is in the table.





using namespace std;

//Constructor for the bloom filter
BloomFilter::BloomFilter(double p, int m, float c, float d){
    //assigning values in the class to their corresponding parameters.
    pr = p;
    numElem = m;
   size = BloomFilterSize(p,m,c);
   //size = 10000;
    bt = new bool[size];
    for(int i = 0 ; i< size; i++){
        bt[i] = false;
    }
    bool notPrime = true;
    bool prime = true;
    k = size;
    //Finding the next prime number after the the bloom filters size
    //Done in the constructor to avoid repeating the calclation.
    while(notPrime){
        prime = true;
        k++;
        for(int i = 2; i < sqrt(k);i++){
            if(k%i == 0){
                prime = false;
            }
        }
        if(prime){
            notPrime = false;
        }
    }
    //Number of hash functions will be the same regardless of scalar 
    //multiplier on the bloom filter size.
    int ans = numHashFunctions(size/c, numElem, d);
    srand (time(NULL));
    int a = 0;
    int b = 0;
    pair<int,int> pa;
    //filling the family hash function with a pair of two random numbers from 1 to k-1 using the rand() method
    //These pairs are used to calculate the hashing function in the hash method.
    for(int i = 0; i < ans; i++){
        a = (rand() % k-1) + 1;
        b = (rand() % k-1) + 1;
        pa = make_pair(a,b);
        hFunc.push_back(pa);
    }
    //Sets the remove hash table size to the next prime number after 10th of the expected entries.
    q = numElem/10;
    notPrime = true;
    prime = true;
    while(notPrime){
        prime = true;
        q++;
        for(int i = 2; i < sqrt(q);i++){
            if(q%i == 0){
                prime = false;
            }
        }
        if(prime){
            notPrime = false;
        }
    }
    ht = new HashTable(q); 
}

//inserts a string into the bloom filter
void BloomFilter::insert(string element){
    //if a string already exists in the bloom filter insert will not do anything
    if(!(find(element))){
        //If an element is added to the bloom filter it has to be removed from the second hash table
        ht->remove(element);
        int index = 0;
        //converts the string to an int so it can be passed into the hash function
        unsigned int elem = strToInt(element);
        //For every hash function, the method will change 1 index in the bloom filter to 1, unless it is already 1.
        //The specific indices are decided by the hashing function.
        for(int i = 0; i < hFunc.size(); i++){
            index = hash(elem,i);
            bt[index] = true;
        }
    }
}

//checks if an element is in the bloom filter
bool BloomFilter::find(string element){
    int index = 0;
    //converts the element into a string for the hash function.
    unsigned int elem = strToInt(element);
    //Will return false if the element exists in the removed hash table
    bool isThere = ht->find(element);
    if(isThere){
        return false;
    }
    //checks if each hash function says the element is in the bloom filter
    //If any of them say it is not then the element doesn't exist in the bloom filter
    for(int i = 0; i < hFunc.size(); i++){
        index = hash(elem,i);
        if(bt[index] == 0){
            return false;
        }
    }
    return true;
}

//removes an element from the bloom filter by adding it to the secondary hash table
void BloomFilter::remove(string element){
    //only adds an element to the hash table if it already exists in the bloom filter.
    //If it is not in the bloom filter, the function doesn't do anything because there 
    //is nothing to remove.
    bool x = find(element);
    if(x){
        ht->insert(element);
    }
}
//bloom filter destructor.
BloomFilter::~BloomFilter(){
    //deletes the bloom filter array and the secondary hash table
    delete[] bt;
    delete ht;
}

//prints out the bloom filter array
//used for testing
void BloomFilter::print(){
    for(int i = 0; i < size; i++){
        cout  << i << "," << bt[i] << " ";
        if(i%10 == 0){
            cout << endl;
        }
    }
}


//Calculates the size of the bloom filter using the expected false positive probability,
//expected number of element entries, and a scalar multiplier.
//This is accomplished using the equation given in the instructions.
int BloomFilter::BloomFilterSize(double p, int m, float c){
    //log(p) = ln p
    double temp = (double(m) * log(p));
    temp = -1*temp;
    double ln = log(2) * log(2);
    int ans = temp/ln;
    ans = ans * c;

    return ans;
}

//hash function for the bloom filter
//The index paramter picks which hash function, from the hash function
// family, will be used on the element paramter.
//Hash function equation = (((ax + b) mod k) mod m)
//a and b are random numbers from 1 to k-1
//x is the element parameter
//k is the highest prime number after the bloom filter size
//m is the bloom filter size
int BloomFilter::hash(unsigned int element, int index){
    //getting the a and b value for the hash function.
    unsigned long long a = hFunc[index].first;
    unsigned long long b = hFunc[index].second;
    unsigned int maxInt = UINT_MAX;
    a = a * element;
    //modding a by the max int value to make sure it will not cause overflow problems.
    a = a % maxInt;
    unsigned long long c = a + b;
    bool notPrime = true;
    c = c % k;
    c = c % size;
    int ans = c % INT_MAX;

    return ans;
}

//Calculating the number of hash functions required for the bloom filter based on the 
//bloom filter size, number of elements, and a scalar.
//This is done using the in class equation.
//log(x) = ln x
int BloomFilter::numHashFunctions(int n, int m, float d){
    double temp = double(n)/m;
    int ans = ((temp * log(2)));
    if(ans == 0){
        ans = 1;
    }
    ans = ans * d;
    return ans;
}

//converts a string into an unsigned int
//Uses the formula given in the instructions:
//x = d[0] * C^0 + d[1]*C^1 + d[n]*C^n
//d = to the string argument
//C is a constant. In this case it is 53 because that is a prime number close to the number of
//letters(upper and lower).
unsigned int strToInt(string element){
    //made as long long to help avoid overflow
    unsigned long long  sum= 0;
    unsigned long long c = 1;
    //The constant
    int mult = 53;
    unsigned int ans = 0;
    //iterates through each character in element and adds the value to the sum.
    for(int i = 0; i < element.size(); i++){
        sum += (c * element[i])%UINT_MAX;
        //setting the constant to C^i. Would have been C^i-1 before this.
        //Starts at C^0
        c = c * mult;
        //modding C down to 32 bits to avoid any overflow.
        c = c % UINT_MAX;
    }
    //Modding the total sum down to 32 bits to match the return type of the function.
    ans = sum % UINT_MAX;
    return ans;
}


//Hash table constructor.
//input is the hash table size.
HashTable::HashTable(int q){
    m = q;
    hashTable = new node*[q];
    for(int i = 0; i < m; i++){
        hashTable[i] = NULL;
    }
    
}


//Hashing function for the hash table.
//function is: x mod m
//x is the int corresponding to the inputed string
// m is the size of the hash table
int HashTable::hash(string element){
    unsigned int num = strToInt(element); 
    return (num % m);
}

//inserts an element into a bucket in the hash table.
//Adds element to the front of that buckets linked list.
void HashTable::insert(string element){
    //Will not insert into the hash table if the element already exists in it.
    if(!find(element)){
        int index = hash(element);
        node* in = new node();
        in->val = element;
        node* temp = hashTable[index];
        hashTable[index] = in;
        in->next = temp;
    }
}

//goes through the hash table bucket assoicated with element
//If element is in the bucket it will remove it from that bucket.
//If it is not there nothing will happen.
void HashTable::remove(string element){
    int index = hash(element);
    node* temp = hashTable[index];
    if(temp == NULL){
        return;
    }
    if(temp->val == element){
       hashTable[index] = temp->next;
       delete temp;
       return;
    }
    node* prev = temp;
    temp = temp->next;
    while(temp != NULL){
        if(temp->val == element){
            prev->next = temp->next;
            delete temp;
            return;
        }
        prev = temp;
        temp = temp->next;
    }
    return;
}

//goes through the hash table bucket assoicated with element
//If element is in the bucket it will return true, otherwise it returns false
bool HashTable::find(string element){
    int index = hash(element);
    node* temp = hashTable[index];
    while(temp != NULL){
        if(temp->val == element){
            return true;
        }
        temp = temp->next;
    }
    return false;
}

//outputs the hash table for testing purposes.
void HashTable::print(){
    for(int i = 0; i< m; i++){
        if(hashTable[i] == NULL){
            cout << 0 << " " << endl;
        }else{
              node* temp = hashTable[i];
             while(temp != NULL){
                cout << temp->val << " -> ";
                temp = temp->next;
             }
             cout << endl;
         }
    }
}

//Goes through each hashtable linked list and deletes it.
//Then deletes the array holding the linked lists.
HashTable::~HashTable(){
    node* temp;
    node* del;
     for(int i =0; i < m; i++){
        temp = hashTable[i];
        del = temp;
        while(temp != NULL){
            temp = temp->next;
            delete del;
            del = temp;
        }
     }
     delete[] hashTable;
}



int main(int argc, char* argv[]){


    vector<string> a;
    string temp;
    ifstream f;
    double p;
    int m;
    float c;
    float d;
    
    //Parsing setup.txt
    f.open(argv[1]);
    if(f.is_open()){
        while(getline(f,temp)){
            a.push_back(temp);
        }
        p = stod(a[0]);
        m = stoi(a[1]);
        c = stof(a[2]);
        d = stof(a[3]);
      //  cout << p;
        f.close();
    }
    BloomFilter b = BloomFilter(p,m,c,d);
    cout << "Experiment for values of: " << endl;
    cout << "p = " << p << endl;
    cout << "c = " <<  c <<endl;
    cout << "d = " << d << endl;
    cout << "q = " << b.q << endl;
    // cout << b.size << endl;
    //  cout << b.hFunc.size() << endl;
    ifstream fa;
    fa.open(argv[4]);
   // getline(fa,temp);
    ifstream in;
    in.open(argv[2]);
   // getline(in,temp);
    ifstream s;
    s.open(argv[3]);
  //  getline(s,temp);
    ifstream r;
    r.open(argv[5]);
   // getline(r,temp);
    double countF = 0;
    double countN = 0;
    double countFTotal = 0;
    double countNTotal = 0;
    vector<string> falseNeg;
    vector<string> falseNegTotal;
    for(int i =0; i < 10; i++){
        countN = 0;
        countF = 0;
        falseNeg.clear();
        //Parsing input.txt
        if(in.is_open()){
            for(int i = 0; i < 1000; i++){
                getline(in,temp);
                b.insert(temp);
            }
        }
        //parsing successfulSearch.txt
        if(s.is_open()){
            for(int i = 0; i < 100; i++){
                getline(s,temp);
                bool x = b.find(temp);
            //   cout << x;
               if(x == false){
                    countN++;
                    countNTotal++;
                }
            }
        }

        
        //parsing failedSearch.txt
        if(fa.is_open()){
            for(int i = 0; i < 100; i++){
                getline(fa,temp);
                bool x = b.find(temp);
                //cout  <<  x;
                if(x == true){
                    countF++;
                    countFTotal++;
                    falseNeg.push_back(temp);
                    falseNegTotal.push_back(temp);
                }
            }
        }
        
        //parsing remove.txt
        if(r.is_open()){
            for(int i = 0; i < 100; i++){
                getline(r,temp);
                b.remove(temp);
            }
            
        }
        cout << "Phase " << i+1 << endl;
        cout << "Number of false negatives: " << endl;
        cout << countN << endl;
        cout << "Number of false positives: " << endl;
        cout << countF << endl;
        cout << "Probability of false positives: " << endl;
        cout << countF/100 << endl;
        cout << "False Positive Elements: " << endl;
        for(int i = 0; i < countF; i++){
            cout << falseNeg[i] << " " << i+1 << endl;
        }
    }
    f.close();
    r.close();
    s.close();
    fa.close();
    //b.print();

    cout << "Number of false negatives: " << endl;
    cout << countNTotal << endl;
    cout << "Number of false positives: " << endl;
    cout << countFTotal << endl;
    cout << "Probability of false positives: " << endl;
    cout << countFTotal/1000 << endl;




    //BloomFilter b = BloomFilter(0.1, 10, 1.0, 1.0);
    // HashTable* h = new HashTable(2);
    //  h->insert("asd");
    // h->insert("asdsd");
    // h->insert("adfsd");
    // h->insert("adfsd");
    // h->insert("adfsd");
    // h->insert("asdfd");
    // h->print();
    // delete h;
    // b.insert("asd");
    // cout << b.find("asd") << endl;
    // b.insert("sdk");
    // b.insert("owjkdo");
    // b.remove("asd");
    // cout << b.find("asd") << endl;
    // b.remove("sdk");
    // b.remove("asd");
    // b.remove("owjkdo");
    // b.print();
    // cout << endl;
    // b.ht->print();
    // int si = b.ht->m;
    // b.ht->print();
    // for(int i = 0; i < si; i++){
    //     if(b.ht->hashTable[i] != NULL){
    //         cout << i << endl;
    //     }
    // }
     //delete b.ht;
    // for(int i = 0; i < si; i++){
    //     if(b.ht == NULL){
    //         cout << "sjd" << endl;
    //     }
    //     if(b.ht->hashTable[i] != NULL){
    //         cout << i << endl;
    //     }
    // }






    // b.insert("asdfg");
    // cout << b.find("asdfg") << endl;
    // b.remove("asdfg");
    // cout << b.find("asdfg") << endl;
    // cout << b.ht->find("asdfg") << endl;
    // b.insert("asdfg");
    // cout << b.find("asdfg") << endl;
    // cout << b.ht->find("asdfg") << endl;


    //b.print();
    





    // HashTable h = HashTable(2);
    // h.insert("2");
    // h.insert("5");
    // h.insert("a");
    // h.insert("3");
    // h.insert("4");
    // h.insert("2");
    // cout << h.find("2") << endl;
    // h.remove("4");
    // cout << h.find("2") << endl;
    return 0;
}
