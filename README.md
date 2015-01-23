## About

This a a porting of BlowFish to SAM4S Xplained Pro

## Objective

The goal is to have a copy of BlowFish that is ready to be used with the SAM4S Xplained Pro.


## Quick Start

This encryption library is fairly straight forward. It has a structure and three functions.

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

## Test Program

A simple test program that encrypt and decrypt using a key. 
The result is display on the Atmel Studio Terminal Window 
---
	  Blowfish_Init (&ctx, (unsigned char*)"TESTKEY", 7);
	  //
	  Blowfish_Encrypt(&ctx, &bL, &bR);
	  printStrF(bL,bR);
	   
	  if (bL == 0x47B8B9ECL && bR == 0xCE55FF90L)
	  printStr("Test encryption OK.\n");
	  else
	  printStr("Test encryption failed.\n");
	  //
	  Blowfish_Decrypt(&ctx, &bL, &bR);
	  printStrF(bL,bR);
	  if (L == bL && R == bR)
	  printStr("Test decryption OK.\n");
	  else
	  printStr("Test decryption failed.\n");
---

## BlowFish Source Code

- https://www.schneier.com/blowfish-download.html 
 
## Credits

-  This port uses Paul Kocher's 1997 work
