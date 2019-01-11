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
#define HSIZE 20
//#define H 3
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
__m128i hashkeys[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashpayloads[HSIZE] __attribute__ ((aligned (128)));;

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

  //if value not present, insert the value
  if(LinearProbeResult>1){
    LinearinsertBegin = clock();
    int insertReturn = LinearInsert(key,LinearProbeResult);
    LinearinsertEnd = clock();
    if(insertReturn != -1){
      LinearInsertTime = LinearInsertTime +  ((long double)LinearinsertEnd - (long double)LinearinsertBegin);      return 1;
    }
    return -1;
  }
  return 1;
}

int LinearProbe(unsigned int key){
   hash(key);
   unsigned int foffset = hashResult[0];
  unsigned int i;  
  int H = 2;
  //check if the key is present with in it's neighborhood
  for(i=foffset; i<HSIZE; i++ ) {
    int *valkey = (int*) &hashkeys[i];
    int *val = (int*) &hashpayloads[i];

    if(i<(foffset+H)) {
      for(int j=0;j<4;j++){
        if(valkey[j] == key) {
          val[j] += 1;
          return 1;  
        }
        else if(valkey[j]==0){
          val[j] = 1;
          valkey[j] = key;          
          return 1;          
        }
      }
    }
    else 
      if((valkey[0]==0)){
        return i*4;
      }
    else  
      if((valkey[1]==0)){
        return (i*4)+1;
      }
    else 
      if((valkey[2]==0)){
        return (i*4)+2;
      }      
    else {
        return (i*4)+3;
      }
      // if((valkey[0]==0)||(valkey[1]==0)||(valkey[2]==0)||(valkey[3]==0))
      //   return i;
    
  }
  return 0;
}

int LinearInsert(unsigned int searchKey, int offNew){

  clock_t begin = clock();

  unsigned int j = offNew;
  unsigned int key = searchKey;
  hash(key);
  unsigned int i = hashResult[1];
     printf("\nkey...\t%d\t%d",key,j);
  int H = 2;
  int cond = i+(H*4);
  while (j>=cond) { //until the key is with in it's neighborhood
     
      int k = j-1;
   int *key1 = (int*) &hashkeys;
    int *pay = (int*) &hashpayloads;
   k = key1[k];

    hash(k);
    k= hashResult[1];
    //printf("%d",k);
    int offj = j/4;
    int offk = k/4;

    int slotj = j%4;
    int slotk = k%4;

    int *valkeyK = (int*) &hashkeys[offk];
    int *valK = (int*) &hashpayloads[offk];
    int *valkeyJ = (int*) &hashkeys[offj];
    int *valJ = (int*) &hashpayloads[offj];

    if(k>=(j-(H*4))) {
      key1[j] = key1[k];
      key1[k]=0;
      pay[j] = pay[k];
      pay[k] = 0;
      // valkeyJ[slotj] = valkeyK[slotk];
      // valJ[slotj] = valK[slotk];
      // valkeyK[slotk] = 0;
      // valK[slotk] = 0;
    }
    j=k;
  }

  int buckj = j/4;
  int slotj = j%4;
  int *valkeyj = (int*) &hashkeys[buckj];
  int *valj = (int*) &hashpayloads[buckj];
     int *key2 = (int*) &hashkeys;
    int *pay2 = (int*) &hashpayloads;
  
  // if(((j<(i+(H*4))) && (valkeyj[slotj]==0))) {
  if(((j<(i+(H*4))) && (key2[j]==0))) {
    // valkeyj[slotj] = key;
    // valj[slotj] =0;
    key2[j]=key;
    pay2[j]=0;
    return 1;
  }
  if(H==HSIZE)
      return -1;
  else {        H++;
  LinearInsert(searchKey, offNew); }

}


int hashCheck(){
  int count = 0;
  int j;
  for(j=0;j<HSIZE;j++){
   int *keys =  (int*) &hashkeys[j];
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
   int *keys =  (int*) &hashpayloads[j];
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
    int *payload =  (int*) &hashpayloads[j];
    int *key =  (int*) &hashkeys[j];
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
   __m128i keys =  hashkeys[j];
   __m128i payloads =  hashpayloads[j];
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
