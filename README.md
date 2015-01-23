## About

This a a porting of BlowFish to SAM4S Xplained Pro

## Objective

The goal have a copy of BlowFish that is ready to be used with the SAM4S Xplained Pro.


## Quick Start

This encryption library is fairly straight forward. It is a structure and three functions.
They are as follows:

## structure
```
typedef struct {
  unsigned long P[16 + 2];
  unsigned long S[4][256];
} BLOWFISH_CTX;
```
## Functions

```
void Blowfish_Init(BLOWFISH_CTX *ctx, unsigned char *key, int keyLen);
void Blowfish_Encrypt(BLOWFISH_CTX *ctx, unsigned long *xl, unsigned long *xr);
void Blowfish_Decrypt(BLOWFISH_CTX *ctx, unsigned long *xl, unsigned long *xr);
```


## BlowFish Source Code

- https://www.schneier.com/blowfish-download.html 
 
## Credits

-  This port uses Paul Kocher's 1997 work
