#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

typedef union {
  __m128i v;
  unsigned int ui;
  signed int si;
  unsigned short us;
  signed short ss;
} vec __attribute__ ((aligned (16)));


#define KEYSPERPAGE 1 
#define REPEATS 2 
#define HSIZE 26

int LinearInsert(unsigned int searchKey, int offNew, int H);
int LinearProbe(unsigned int key, int H);
int SIMDProbe(unsigned int key, int H);
int SIMDInsert(unsigned int searchKey,unsigned int j, int H);
void print128_num(__m128i var);


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
  __m128i morekeys;
  __m128i payloads;
  __m128i indexes;
} entry8;

//#if (BFACTOR==4)
//typedef entry4 entry;
//#else
typedef entry8 entry;
//#endif

//entry hashtable[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashkeys[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashpayloads[HSIZE] __attribute__ ((aligned (128)));;



  unsigned int hash1(int key){
    return (13*key)%HSIZE;
  } 

  unsigned int hash2(int key){
    return 4*((13*key)%HSIZE);
  }


int LinearProbeInsert(unsigned int key){
   int H = 2;
  LinearProbeBegin = clock();
  int LinearProbeResult = LinearProbe(key,H);

  //if value not present, insert the value
  if(LinearProbeResult>1){
    LinearinsertBegin = clock();
    int insertReturn = LinearInsert(key,LinearProbeResult,H);
    LinearinsertEnd = clock();
    if(insertReturn != -1){
      LinearInsertTime = LinearInsertTime +  ((long double)LinearinsertEnd - (long double)LinearinsertBegin);      return 1;
    }
    return -1;
  }
  LinearProbeEnd = clock();
  LinearProbeTime = LinearProbeTime + ((long double)LinearProbeEnd - (long double)LinearProbeBegin);
  return 1;
}

int LinearProbe(unsigned int key, int H){
   
  unsigned int foffset = hash1(key);
  unsigned int i;  
  //check if the key is present with in it's neighborhood
  for(i=foffset; i<HSIZE; i++) {
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
      if((valkey[0]==0)){ //to send the exact the bucket and slot of empty location to insert function
        return i*4;
      }
      if((valkey[1]==0)){
        return (i*4)+1;
      }
      if((valkey[2]==0)){
        return (i*4)+2;
      }      
      if ((valkey[3]==0)){
        return (i*4)+3;
      }
  }
  return 0;
}

int LinearInsert(unsigned int searchKey, int offNew, int H){


  clock_t begin = clock();

 
  unsigned int key = searchKey;

  unsigned int i = hash2(key);
  unsigned int j = offNew;

  int k;
  int cond = i+(H*4);
  while (j>=cond) { //until the key is with in it's neighborhood
     
    k = j-1;
    int *val = (int*) &hashkeys;
    int *pay = (int*) &hashpayloads;
   int q = val[k];

    q= hash2(q);
    if(k<(q+(H*2))) {
      val[j] = val[k];
      pay[j] = pay[k];
      val[k]=0;      
      pay[k] = 0;
    }    
    j=k;
  }

  int *val2 = (int*) &hashkeys;
  int *pay = (int*) &hashpayloads;
  if(((j<(i+(H*4))) && (val2[j]==0))) {
    val2[j]=key;
    pay[j]=1;
    return 0;
  }
    
  if(H==HSIZE)
    return -1;
   else {        
    H++;
    LinearInsert(searchKey, offNew, H); 
  }
  return -1;
}

int SIMDProbeInsert(unsigned int key){
  SIMDProbeBegin = clock();
    int H = 2;
  int SIMDProbeResult = SIMDProbe(key,H);

  //if value not present, insert the value
  if(SIMDProbeResult>1){
    SIMDinsertBegin = clock();
    int insertReturn = SIMDInsert(key,SIMDProbeResult,H);
    SIMDinsertEnd = clock();
    if(insertReturn != -1){
      SIMDInsertTime = SIMDInsertTime +  ((long double)SIMDinsertEnd - (long double)SIMDinsertBegin);
      return 1;
    }
    return -1;
  }
  SIMDProbeEnd = clock();
  SIMDProbeTime = SIMDProbeTime + ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);  
  return 1;
}

int SIMDProbe(unsigned int key, int H)
{
    SIMDProbeBegin = clock();

    unsigned int foffset = hash1(key);

    __m128i mask0 = _mm_setzero_si128();
    __m128i tmp;
    __m128i slot;
    __m128i k;
 
  for(int i=foffset; i<HSIZE; i++) {

    slot = hashkeys[i];
    
      //Get the values of the offset into registers
    if(i<(foffset+H)) {


      //Key value replicated into SIMD registers
      k = _mm_set_epi32(key,key,key,key);

    //Compare the values in the registers and add to payload
    //Compare registers
      tmp = _mm_cmpeq_epi32(k,slot);

    //Check if any value is 1
      if(_mm_movemask_epi8(tmp)){
      
      //Add to Payload
        hashpayloads[i] = _mm_sub_epi32(hashpayloads[i],tmp);      
        return 1;
      }
    
      __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot);
      int resMove = _mm_movemask_epi8(zeroPos);
      if(resMove){

        //Insert values
        //shift the values
        hashpayloads[i] = _mm_bslli_si128(hashpayloads[i],4);
        hashkeys[i] = _mm_bslli_si128(hashkeys[i],4);
      
        //Place the key in first position
        int *valkey = (int*) &hashkeys[i];
        int *val = (int*) &hashpayloads[i];
        val[0] = 1;
        valkey[0] = key;
        return 1;
      }
    }
    else  {
       __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot);
       int *pt = (int*) &zeroPos;
       int resMove = _mm_movemask_epi8(zeroPos);    
       if (resMove) {
        if(pt[0]==-1) { //to send the exact the bucket and slot of empty location to insert function
          return i*4;
        }
        if(pt[1]==-1) {
          return (i*4)+1;
        }
        if(pt[2]==-1) {
          return (i*4)+2;
        }
        if(pt[3]==-1){
          return (i*4)+3;
        }
      }
    }
  }
  return 0;
}


int SIMDInsert(unsigned int searchKey,unsigned int jNew, int H){
  clock_t begin = clock();
  
  unsigned int key = searchKey;
  unsigned int i = hash2(key);
  unsigned int j = jNew;
  int *indexes;

  __m128i tmpJ;
  __m128i tmpI;
  __m128i tmpH;
  __m128i tmpA;
  __m128i tmpS;
  __m128i tmpQ;     
  unsigned int k;
  int l=0;

indexes = (int *)calloc(100,sizeof(int));

for(k = j; k>=i+(H*4);k=k-4)
{
    int *keyJ = (int*) &hashkeys[0];
    tmpQ = _mm_set_epi32(hash2(keyJ[k-3]),hash2(keyJ[k-2]),hash2(keyJ[k-1]),hash2(keyJ[k]));    
    tmpJ = _mm_set_epi32(k-3,k-2,k-1,k);    
    tmpH = _mm_set_epi32(H*4,H*4,H*4,H*4);
    tmpI = _mm_set_epi32(i,i,i,i);

    tmpA = _mm_add_epi32(tmpI,tmpH);
    tmpS = _mm_sub_epi32(tmpQ,tmpA);

    __m128i zero = _mm_set1_epi32(-2); //all the indexes within neighborhood will return negative values on j-(i+H) 
                                       // so we consider all indexes until -1, that is to reach a value inside the neighborhood
    __m128i c = _mm_cmpgt_epi32(tmpS,zero);
    int *checkey = (int *) &c;

      int *val = (int*) &tmpJ;
    if(checkey[0] == -1)
    {
      indexes[l++] = val[0]; 
    }
    if(checkey[1] == -1)
    {
      indexes[l++] = val[1]; 
    }
    if(checkey[2] == -1)
    { 
      indexes[l++] = val[2]; 
    }
    if(checkey[3] == -1)
    {
      indexes[l++] = val[3]; 
    } 
  }


  for(int i=0;i<l;i=i+3) {
    
    __m128i id = _mm_loadl_epi64((__m128i *) &indexes[i]);
    int *valI = (int*) &id; //if the id contains less that 4 indexes, set the remaining to -1
    if(valI[0] == 0) {
      valI[0] = -1;
    }
    if(valI[1] == 0) {
      valI[1] = -1;
    }
    if(valI[2] == 0) {
      valI[2] = -1;
    }
    if(valI[3] == 0) {
      valI[3] = -1;
    }
    
    int *key = (int*) &hashkeys[0];
    int *pay = (int*) &hashpayloads[0];

    __m128i values = _mm_i32gather_epi32 (key, id, 4);
    __m128i payloads = _mm_i32gather_epi32 (pay, id, 4);

    __m128i scatteredkeys = _mm_bsrli_si128(values,4);
    __m128i scatteredpayloads = _mm_bsrli_si128(payloads,4);

     int *val = (int*) &id;
     int *sck = (int*) &scatteredkeys;
     int *scp = (int*) &scatteredpayloads;
     key[val[0]] = sck[0];
     key[val[1]] = sck[1];
     key[val[2]] = sck[2];
     key[val[3]] = sck[3];

     pay[val[0]] = scp[0];
     pay[val[1]] = scp[1];
     pay[val[2]] = scp[2];
     pay[val[3]] = scp[3];
   
  }

  int *ptk = (int*) &hashkeys[0];
  if(ptk[indexes[l-1]]==0) { //insert key and payload inside neighborhood
    int *ptp = (int*) &hashpayloads;
    ptk[indexes[l-1]] = key;
    ptp[indexes[l-1]] = 1;
    return 0;
  }
  if(H==HSIZE){
    return -1;
  }
  else {        
   H++;
    SIMDInsert( searchKey, jNew, H); 
  }
  return -1;
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

