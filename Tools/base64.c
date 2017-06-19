#include "base64.h"
#include "string.h"
#include "cmsis_os.h"


const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static void *(*b64_malloc)(size_t sz) = pvPortMalloc;
static void (*b64_free)(void *ptr) = vPortFree;

static uint8_t GetIndex(uint8_t num)
{
  uint8_t len;
  len=strlen(base);
  for(int i=0;i<len;i++)
  {
    if(base[i]==num)
      return i;
  }
  return 255;
}


uint8_t* b64_decode (uint8_t *in)
{
  uint32_t len=strlen(in);
  uint32_t mlen=0;
  uint32_t size=0;
  if(len%4!=0)
    return NULL;
  uint8_t *out;
  mlen=len/4*3;//maybe have '='
  if(in[len-1]=='=')
    mlen--;
  if(in[len-2]=='=')
    mlen--;
  out = b64_malloc(mlen+1);//add '\0'
  if(out==NULL)
    return NULL;
  
  uint32_t b[4];
  for (int i = 0; i < len; i += 4) 
  {
    b[0] =GetIndex(in[i]);
    b[1] =GetIndex(in[i+1]);
    b[2] =GetIndex(in[i+2]);
    b[3] =GetIndex(in[i+3]);
    out[size++]=(uint8_t)((b[0]<<2)|(b[1]>>4));
    if (b[2] < 64)
    {
      out[size++] = (uint8_t) ((b[1] << 4) | (b[2] >> 2));
                if (b[3] < 64)  
                {
                    out[size++] = (uint8_t) ((b[2] << 6) | b[3]);
                }
     }
  }
  out[size++]='\0';
  return out;
}

uint8_t* b64_encode (uint8_t *in)
{
  uint8_t *out;
  uint32_t len=strlen(in);
  uint32_t mlen;
  // alloc
  if(len%3)
  {
    mlen=(len/3)*4+4;
  }
  else
  {
    mlen=(len/3)*4;
  }
  out = b64_malloc(mlen+1);//add '\0'
  if(out==NULL)
    return NULL;
  int b;
  int size=0;
  for(int i=0;i<len;i+=3)
  {
    b=(in[i]&0xfc)>>2;
    out[size++]=base[b];
    b=(in[i]&0x03)<<4;
    if(i+1<len)//
    {
       b |= (in[i + 1] & 0xF0) >> 4;
       out[size++]=base[b];
       b = (in[i + 1] & 0x0F) << 2;
       if(i+2<len)
       {
          b |= (in[i + 2] & 0xC0) >> 6;
          out[size++]=base[b];
          b = in[i + 2] & 0x3F;
          out[size++]=base[b];
       }
       else
       {
         out[size++]=base[b];
         out[size++]= '=';
       }
    }
    else
    {
      out[size++]=base[b];
      out[size++]= '=';
      out[size++]= '=';
    }
  }
  out[size++]='\0';
  return out;
}