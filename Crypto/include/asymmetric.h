#pragma once

#include <stdio.h>
#include <ctime>

#include <ipp.h>
#include <ippcp.h>

#include "ippcp_bignumber.h"

#define N_TRIAL   10
#define MAX_TRIAL 25

class RSA_Crypt
{
  public:
    // Variables
    int bitsize = 0;

    // Functions
    RSA_Crypt(const int bitsize,
              Ipp8u *private_key = nullptr,
              size_t privateSize = 0,
              Ipp8u *public_key  = nullptr,
              size_t publicSize  = 0);
    IppStatus setKey(int key_type, const Ipp8u *key, int keySize);
    IppStatus encryptMessage(const Ipp8u *msg, int lenmsg, Ipp8u *ciphertext, Ipp8u *label = nullptr, int lenlabel = 0);
    IppStatus decryptMessage(const Ipp8u *ciphertext,
                             Ipp8u *msg,
                             int &lenmsg,
                             Ipp8u *label = nullptr,
                             int lenlabel = 0);
    IppStatus getKey(int key_type, Ipp8u *key, int keysize);

#ifdef _DEBUG
    void printKeys();
    IppStatus readKeys(const std::string filepath);
    IppStatus saveKeys(const std::string filepath);
#endif
    ~RSA_Crypt();

  private:
    // Variables
    IppsRSAPrivateKeyState *privateKey = nullptr;
    IppsRSAPublicKeyState *publicKey   = nullptr;
    IppsPrimeState *pPG;
    IppsPRNGState *pRNG;
    Ipp32u *seed;
    Ipp8u *buffer;
    int bitsP, bitsQ;

    // Functions
    inline void generate_PrimeGenerator(int maxbits, IppsPrimeState *&pPG);
    inline void generate_RandomGenerator(int seedbits, IppsPRNGState *&pRNG, IppsBigNumState *seed = 0);
    inline Ipp32u *rand32(int size);
};