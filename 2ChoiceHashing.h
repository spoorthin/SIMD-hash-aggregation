#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emmintrin.h>
#include <smmintrin.h>

 typedef union {
  __m128i v;
  unsigned int ui[4];
  signed int si[4];
  unsigned short us[8];
  signed short ss[8];
} vec __attribute__ ((aligned (16)));


#define KEYSPERPAGE 1 
#define REPEATS 2 
#define HSIZE 330000
int LinearInsert(unsigned int searchKey);


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

entry hashtable[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashkeys[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashpayloads[HSIZE] __attribute__ ((aligned (128)));;


void print128_num(__m128i var){
    int *val = (int*) &var;
    printf("%i\t%i\t%i\t%i", 
           val[0], val[1], val[2], val[3]);
}


unsigned int hash1(int key){
  return ((unsigned long)((unsigned int)1300000077*key)* HSIZE)>>32; 
}

unsigned int hash2(int key){
  return ((unsigned long)((unsigned int)1145678917*key)* HSIZE)>>32;
}

  __m128i m0; /* original multipliers - we'll shift them to get m for efficiency */

  __m128i tbsize;


  vec m;
  vec tb; 

  void setM(){
    m.ui[0] = 1300000077;
    m.ui[2] = 1145678917;

    m0 = m.v;
    tbsize = _mm_set_epi32(HSIZE,HSIZE,HSIZE,HSIZE);  
  }

__m128i hash(__m128i k)
{
  __m128i h;


  h = _mm_mul_epu32(m0,k);  // hash values are in slots 0 and 2 
  // now multiply by table size; don't need to zero out parts of h.v because mult only looks at lower 32 bits

  h = _mm_mul_epu32(h,tbsize);

  //print128_num(h);  
  // result is in slots 1 and 3 (32-bit values), i.e., slots 2 and 6 (16-bit values)
  return h;
}

#if (HSIZE <= 3300)
#define kextract(hvec,size) _mm_extract_epi16(hvec,size)
#else
#define kextract(hvec,size) (_mm_extract_epi16(hvec,size) | (_mm_extract_epi16(hvec,size+1)<<16))
// The following is not as good as the above
//#define kextract(hvec,size) (((vec)hvec).ui[size/2])
#endif

int ScalarProbe(unsigned int key){
  LinearProbeBegin = clock();

  //Hash the key
  unsigned int foffset1 = hash1(key);
  unsigned int foffset2 = hash2(key);

  unsigned int foffsettemp1 = foffset1;
  unsigned int foffsettemp2 = foffset2;

  while((foffset1 != (foffsettemp2-1)) &&  (foffset2 != (foffsettemp1-1)))
  {
    //Get the values of the offset 
    int *valkey1 = (int*) &hashkeys[foffset1];
    int *val1 = (int*) &hashpayloads[foffset1];

    int *valkey2 = (int*) &hashkeys[foffset2];
    int *val2 = (int*) &hashpayloads[foffset2];
    int i;

    //search for key in both buckets and aggregate payload
    for (i=0;i<4;i++) {
      if((valkey1[i]==key)||(valkey2[i]==key)){
        val1[i] += (valkey1[i] == key);
        val2[i] += (valkey2[i] == key);  
        LinearProbeEnd = clock();
        ScalarTime = ScalarTime +  ((long double)LinearProbeEnd - (long double)LinearProbeBegin);
        return 1;
      }
       if((valkey1[i]==0)||(valkey2[i]==0)){
        if(valkey1[i]==0)
        { 
          val1[i] = 1;
          valkey1[i] = key;
          LinearProbeEnd = clock();
          ScalarTime = ScalarTime +  ((long double)LinearProbeEnd - (long double)LinearProbeBegin);
          return 1;
        }

        if(valkey2[i]==0) 
        {
          val2[i] = 1;
          valkey2[i] = key;
          LinearProbeEnd = clock();
          ScalarTime = ScalarTime +  ((long double)LinearProbeEnd - (long double)LinearProbeBegin);
          return 1;
        }
      }
    }
  
    foffset1 = (foffset1++)%HSIZE;
    foffset2 = (foffset2++)%HSIZE;
  }
  return -1;
}

int VectorProbe(unsigned int key){
  SIMDProbeBegin = clock();

  __m128i k;
  __m128i h;

  k=_mm_cvtsi32_si128(key); /* replicate key into vector */
  k=_mm_shuffle_epi32(k,0);

  h = hash(k);

  unsigned int foffset1 = kextract(h,2);
  unsigned int foffset2 = kextract(h,6);
  __m128i mask0 = _mm_setzero_si128();
 __m128i mask1 = _mm_setzero_si128();  
  __m128i tmp1;
  __m128i tmp2;
  register __m128i slot1;
  register __m128i slot2;
  register __m128i r;
  unsigned int foffsettemp1 = foffset1;
  unsigned int foffsettemp2 = foffset2;

  while((foffset1 != (foffsettemp2-1)) &&  (foffset2 != (foffsettemp1-1))) {
  //Get the values of the offset into registers
    slot1 = hashkeys[foffset1];
    slot2 = hashkeys[foffset2];

    //Compare the values in the registers and add to payload
    //Compare registers
    tmp1 = _mm_cmpeq_epi32(k,slot1);
    tmp2 = _mm_cmpeq_epi32(k,slot2);

    //Check if any value is 1
    if(_mm_movemask_epi8(tmp1)){
      
      //Add to Payload
      hashpayloads[foffset1] = _mm_sub_epi32(hashpayloads[foffset1],tmp1);      
      SIMDProbeEnd = clock();
      SIMDTime = SIMDTime +  ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);      
      return 1;
    }
    if(_mm_movemask_epi8(tmp2)){
      
      //Add to Payload
      hashpayloads[foffset2] = _mm_sub_epi32(hashpayloads[foffset2],tmp2);      
      SIMDProbeEnd = clock();
      SIMDTime = SIMDTime +  ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);      
      return 1;
    }
    //Check if there are zeros
    __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot1);
    __m128i zeroPosn = _mm_cmpeq_epi32(mask1,slot2);
    int resMove1 = _mm_movemask_epi8(zeroPos);
    int resMove2 = _mm_movemask_epi8(zeroPosn);  

    if(resMove1||resMove2){
      if (resMove1) {
        //Insert values
        //shift the values
        hashpayloads[foffset1] = _mm_bslli_si128(hashpayloads[foffset1],4);
        hashkeys[foffset1] = _mm_bslli_si128(hashkeys[foffset1],4);
      
        //Place the key in first position
        int *valkey = (int*) &hashkeys[foffset1];
        int *val = (int*) &hashpayloads[foffset1];
        val[0] = 1;
        valkey[0] = key;
        SIMDProbeEnd = clock();
        SIMDTime = SIMDTime +  ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);        
        return 1;
      }
      if (resMove2) {
        //Insert values
        //shift the values
        hashpayloads[foffset2] = _mm_bslli_si128(hashpayloads[foffset2],4);
        hashkeys[foffset2] = _mm_bslli_si128(hashkeys[foffset2],4);
      
        //Place the key in first position
        int *valkey = (int*) &hashkeys[foffset2];
        int *val = (int*) &hashpayloads[foffset2];
        val[0] = 1;
        valkey[0] = key;
        SIMDProbeEnd = clock();
        SIMDTime = SIMDTime +  ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);        
        return 1;
      }
    }

    foffset1 = (foffset1++)%HSIZE;
    foffset2 = (foffset2++)%HSIZE;
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