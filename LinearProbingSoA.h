#include <stdio.h>
#include <stdlib.h>
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
#define HSIZE 33000000

//Clock Variables

clock_t SIMDProbeBegin, SIMDProbeEnd, LinearProbeBegin, LinearProbeEnd;

static long double SIMDTime, ScalarTime;

//32 bit keys and 32 bit payloads
typedef struct {
  __m128i keys;
  __m128i payloads;
} entry4;

typedef struct {
  __m128i keys;
  __m128i morekeys;
  __m128i payloads;
  __m128i morepayloads;
} entry8;

#if (BFACTOR==4)
typedef entry4 entry;
#else
typedef entry8 entry;
#endif

__m128i hashkeys[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashpayloads[HSIZE] __attribute__ ((aligned (128)));;

void print128_num(__m128i var){
    int *val = (int*) &var;
    printf("%i\t%i\t%i\t%i", 
           val[0], val[1], val[2], val[3]);
}

//32 bit keys and 32 bit payloads
 unsigned int hash(int key){
  
  return ((unsigned long)((unsigned int)1300000077*key)* HSIZE)>>32;  

}

__inline int VectorProbe(unsigned int key){
  SIMDProbeBegin = clock();
  //Hash the key
  unsigned int foffset = hash(key);
  __m128i mask0 = _mm_setzero_si128();
  __m128i tmp;
  register __m128i slot;
  register __m128i k;

  while(foffset<HSIZE){
  //Get the values of the offset into registers
    slot = hashkeys[foffset];

    //Key value replicated into SIMD registers
    k = _mm_set_epi32(key,key,key,key);

    //Compare the values in the registers and add to payload
    //Compare registers
    tmp = _mm_cmpeq_epi32(k,slot);
    //Check if any value is 1
    
    if(_mm_movemask_epi8(tmp)){
      
      //Add to Payload
      hashpayloads[foffset] = _mm_sub_epi32(hashpayloads[foffset] ,tmp);      
      return 1;
    }
    
    //Check if there are zeros
    __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot);
    int resMove = _mm_movemask_epi8(zeroPos);
    if(resMove){
      
      //Insert values
      //shift the values
      hashpayloads[foffset] = _mm_bslli_si128(hashpayloads[foffset],4);
      hashkeys[foffset] = _mm_bslli_si128(hashkeys[foffset],4);
      
      //Place the key in first position
      int *valkey = (int*) &hashkeys[foffset];
      int *val = (int*) &hashpayloads[foffset];
      val[0] = 1;
      valkey[0] = key;
      SIMDProbeEnd = clock();
      SIMDTime = SIMDTime +  ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);
      return 2;
    }

    foffset = (foffset++)%HSIZE;
  }
  
  return -1;

}


//Linear Probing starts here
__inline int ScalarProbe(unsigned int key){
  LinearProbeBegin = clock();
  //Hash the key
  unsigned int foffset = hash(key);

  int count =0;

  while(foffset<HSIZE){
    
    //Get the values of the offset 
    int *valkey = (int*) &hashkeys[foffset];
    int *val = (int*) &hashpayloads[foffset];
    int i;

    //Compare the values and add to payload
    //For Loop Method
    for(i=0;i<4;i++){
      val[i] += (valkey[i] == key); 
    }

    if((valkey[0]==key)||(valkey[1]==key)||(valkey[2]==key)||(valkey[3]==key))
        return 1;
    
    //Check if there are zeros
    if((valkey[0]==0)||(valkey[1]==0)||(valkey[2]==0)||(valkey[3]==0)){
      
      //Insert values
      //shift the values
      //Place the key in first position  
      //Moving payload
      
      val[3] = val[2];
      val[2] = val[1];
      val[1] = val[0];
      val[0] = 1;

      //Moving keys
      valkey[3] = valkey[2];
      valkey[2] = valkey[1];
      valkey[1] = valkey[0];
      valkey[0] = key;
      LinearProbeEnd = clock();
      ScalarTime = ScalarTime +  ((long double)LinearProbeEnd - (long double)LinearProbeBegin);
      return 2;
    }
    foffset = (foffset++)%HSIZE;
  }
  
  return -1;

}

void HashLookup(){
  int j;
 // printf("|-------------------------------|\n");
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

long double getScalarTime(){
  return ScalarTime;
}

long double  getSIMDTime(){
  return SIMDTime;
}

void clearClocks(){
  ScalarTime=0;
  SIMDTime=0;
}