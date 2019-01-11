#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emmintrin.h>
#include <smmintrin.h>

typedef union {
  __m128i v;
  unsigned int ui;
  signed int si;
  unsigned short us;
  signed short ss;
} vec __attribute__ ((aligned (16)));


#define KEYSPERPAGE 1 
#define REPEATS 2 
#define HSIZE 15
#define H 3
int LinearInsert(unsigned int searchKey, int offNew);
int LinearProbe(unsigned int key);
//Clock Variables




clock_t SIMDinsertBegin, SIMDinsertEnd, SIMDProbeBegin, SIMDProbeEnd, LinearinsertBegin, LinearinsertEnd, LinearProbeBegin, LinearProbeEnd;

static long double SIMDProbeTime, SIMDInsertTime, LinearProbeTime, LinearInsertTime;

//32 bit keys and 32 bit payloads
typedef struct {
  __m128i keys;
  __m128i payloads;
} entry4;

typedef struct {
  __m128i keys;
 // __m128i morekeys;
  __m128i payloads;
 // __m128i indexes;
} entry8;

//#if (BFACTOR==4)
//typedef entry4 entry;
//#else
typedef entry8 entry;
//#endif

entry hashtable[HSIZE] __attribute__ ((aligned (128)));;

 unsigned int hashResult[2];
 void hash(int key){
    hashResult[0] =(13*key)%HSIZE;
    hashResult[1] =4*((13*key)%HSIZE);
  }

int LinearProbeInsert(unsigned int key){
  LinearProbeBegin = clock();
  int LinearProbeResult = LinearProbe(key);
  LinearProbeEnd = clock();
  LinearProbeTime = LinearProbeTime + ((long double)LinearProbeEnd - (long double)LinearProbeBegin);
 // printf("yes!%d\n",LinearProbeResult);
  //if value not present, insert the value
  if(LinearProbeResult>1){
    LinearinsertBegin = clock();
    int insertReturn = LinearInsert(key,LinearProbeResult);
    //printf("yess..%d",insertReturn);
    LinearinsertEnd = clock();
    if(insertReturn != -1){
      LinearInsertTime = LinearInsertTime +  ((long double)LinearinsertEnd - (long double)LinearinsertBegin);
    //  printf("here...%LF\n",LinearInsertTime );
      return 1;
    }
    return -1;
  }
  return 1;
}

int LinearProbe(unsigned int key){
   hash(key);
   unsigned int foffset = hashResult[0];
  unsigned int i;  

  //check if the key is present with in it's neighborhood
  for(i=foffset; i<HSIZE; i++ ) {
    int *valkey = (int*) &hashtable[i].keys;
    int *val = (int*) &hashtable[i].payloads;

    if(i<(foffset+H)) {
      for(int j=0;j<4;j++){
        if(valkey[j] == key) {
          val[j] += 1;
          return 1;  
        }
        else if(valkey[j]==0){
          //printf("inserted\t%d\n",key);
          val[j] = 1;
          valkey[j] = key;          
          return 1;          
        }
      }
    }
    else {
      if((valkey[0]==0)||(valkey[1]==0)||(valkey[2]==0)||(valkey[3]==0))
        return i;
    }
    //if key is present, aggregate payload and return 1

  }
  return 0; // if key is not present return 0 
}

int LinearInsert(unsigned int searchKey, int offNew){

  clock_t begin = clock();
   
  unsigned int j = offNew*4;
  unsigned int key = searchKey;
  hash(key);
  unsigned int i = hashResult[1];

  int cond = i+(H*4);
  while (j>=cond) { //until the key is with in it's neighborhood
   
    int k = j-1;

    int offj = j/4;
    int offk = k/4;

    int slotj = j%4;
    int slotk = k%4;

    int *valkeyK = (int*) &hashtable[offk].keys;
    int *valK = (int*) &hashtable[offk].payloads;
    int *valkeyJ = (int*) &hashtable[offj].keys;
    int *valJ = (int*) &hashtable[offj].payloads;

    if(k>=(j-(H*4))) {
      valkeyJ[slotj] = valkeyK[slotk];
      valJ[slotj] = valK[slotk];
      valkeyK[slotk] = 0;
      valK[slotk] = 0;
    }
    j=k;
  }

  int buckj = j/4;
  int slotj = j%4;
  int *valkeyj = (int*) &hashtable[buckj].keys;
  int *valj = (int*) &hashtable[buckj].payloads;
  
  if(((j<(i+(H*4))) && (valkeyj[slotj]==0))) {
    valkeyj[slotj] = key;
    valj[slotj] =0;
    return 1;
  }
  return -1;
}


int hashCheck(){
  int count = 0;
  int j;
  for(j=0;j<HSIZE;j++){
   int *keys =  (int*) &hashtable[j].keys;
   if(keys[0]!=0)
      count++;
    if(keys[1]!=0)
      count++;
    if(keys[2]!=0)
      count++;
    if(keys[3]!=0)
      count++;
  } 
  // printf("here =%d\n", count);
 return count;
}

int addValues(){
  int count = 0;
  int j;
  for(j=0;j<HSIZE;j++){
   int *keys =  (int*) &hashtable[j].payloads;
   if(keys[0]!=0)
      count+=keys[0];
    if(keys[1]!=0)
      count+=keys[1];
    if(keys[2]!=0)
      count+=keys[2];
    if(keys[3]!=0)
      count+=keys[3];
  } 
 return count;

}

void clearHash(){
  int j;
  for(j=0;j<HSIZE;j++){
    int *payload =  (int*) &hashtable[j].payloads;
    int *key =  (int*) &hashtable[j].keys;
    payload[0] = 0;
    payload[1] = 0;
    payload[2] = 0;
    payload[3] = 0;
    key[0] = 0;
    key[1] = 0;
    key[2] = 0;
    key[3] = 0;

  }
}

void print128_num(__m128i var)
{
    int *val = (int*) &var;
    printf("%i\t%i\t%i\t%i", 
           val[0], val[1], val[2], val[3]);
}

void HashLookup(){
  int j;
  printf("|-------------------------------|\n");
  printf("|----TABLE-------|\n");
  for(j=0;j<HSIZE;j++){
   __m128i keys =  hashtable[j].keys;
   __m128i payloads =  hashtable[j].payloads;
   printf("|-------------------------------|\n|");
   print128_num(keys);
   printf("\t|\n|");
   print128_num(payloads);
   printf("\t|\n");
  }
  printf("|-------------------------------|\n");
}


long double getSIMDTime(){
  return SIMDInsertTime;
}

long double getSIMDProbeTime(){
  return SIMDProbeTime;
}

long double getLinearTime(){
  return LinearInsertTime;
}

long double getLinearProbeTime(){
  return LinearProbeTime;
}

void clearClocks(){
  SIMDInsertTime = 0;
  SIMDProbeTime = 0;
  LinearInsertTime = 0;
  LinearProbeTime = 0;
}
