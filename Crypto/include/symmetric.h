#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctime>
#include <stdexcept>

#include <ipp.h>
#include <ippcp.h>

/// Size of ctr context in bytes
#define CRYPT_CTR_SIZE 16
/// Size of ctr context in bytes
#define AES_CTR_SIZE 16
/// Size of ctr context in bytes
#define SMS4_CTR_SIZE 16

class AES_Crypt
{
    AES_Crypt(Ipp8u *pkey = nullptr, size_t keyLen = 256);
    IppStatus setKey(const Ipp8u *key, size_t keyLen);
    IppStatus resetCtr(Ipp8u *ctr = nullptr, int ctrBitLen = 0);
    IppStatus encryptMessage(const Ipp8u *msg, int lenmsg, Ipp8u *ciphertext, Ipp8u *ctr = nullptr, int ctrBitLen = 0);
    IppStatus decryptMessage(const Ipp8u *ciphertext, Ipp8u *msg, int &lenmsg, Ipp8u *ctr = nullptr, int ctrBitLen = 0);
    ~AES_Crypt() = default;

  private:
    size_t keyLen    = 0;
    IppsAESSpec *key = nullptr;
    Ipp8u *ctr       = nullptr;

    inline Ipp8u *rand8(int size);
};

class SMS4_Crypt
{
    // Functions
    SMS4_Crypt(Ipp8u *pkey = nullptr, size_t keyLen = 256);
    IppStatus setKey(const Ipp8u *key, size_t keyLen);
    IppStatus resetCtr(Ipp8u *ctr = nullptr, int ctrBitLen = 0);
    IppStatus encryptMessage(const Ipp8u *msg, int lenmsg, Ipp8u *ciphertext, Ipp8u *ctr = nullptr, int ctrBitLen = 0);
    IppStatus decryptMessage(const Ipp8u *ciphertext, Ipp8u *msg, int &lenmsg, Ipp8u *ctr = nullptr, int ctrBitLen = 0);
    ~SMS4_Crypt();

  private:
    size_t keyLen     = 0;
    IppsSMS4Spec *key = nullptr;
    Ipp8u *ctr        = nullptr;

    inline Ipp8u *rand8(int size);
};