#include "symmetric.h"

AES_Crypt::AES_Crypt(Ipp8u *pkey, size_t keyLen)
{
    IppStatus status = ippStsNoErr;
    int ctxSize      = 0;

    // Init context
    status = ippsAESGetSize(&ctxSize);
    if (status != ippStsNoErr)
        throw std::runtime_error(ippGetStatusString(status));
    this->key = (IppsAESSpec *)(new Ipp8u[ctxSize]);
    this->ctr = new Ipp8u[AES_CTR_SIZE];
    memset(this->ctr, 1, AES_CTR_SIZE);

    if (pkey == nullptr)
        pkey = rand8(keyLen / 8);

    status = ippsAESInit(pkey, keyLen / 8, this->key, ctxSize);
    if (status != ippStsNoErr)
        throw std::runtime_error(ippGetStatusString(status));

    // Since no throw set key length
    this->keyLen = keyLen;
}

IppStatus AES_Crypt::setKey(const Ipp8u *pkey, size_t keyLen)
{
    IppStatus status;
    status = ippsAESSetKey(pkey, keyLen / 8, this->key);

    if (status == ippStsNoErr)
        this->keyLen = keyLen;

    return status;
}

IppStatus AES_Crypt::resetCtr(Ipp8u *ctr, int ctrBitLen)
{
    if (ctrBitLen > AES_CTR_SIZE)
        return ippStsErr;
    if (this->ctr)
        delete this->ctr;

    this->ctr = new Ipp8u[AES_CTR_SIZE];
    if (!ctr)
        memset(this->ctr, 1, AES_CTR_SIZE);
    else
        memcpy(this->ctr + AES_CTR_SIZE - (ctrBitLen / 8), ctr, ctrBitLen / 8);

    return ippStsNoErr;
}

IppStatus AES_Crypt::encryptMessage(const Ipp8u *msg, int lenmsg, Ipp8u *ciphertext, Ipp8u *ctr, int ctrBitLen)
{
    if (ctr == nullptr)    // If ctr not passed use internal ctr
        return ippsAESEncryptCTR(msg, ciphertext, lenmsg, this->key, this->ctr, AES_CTR_SIZE * 8);
    else
        return ippsAESEncryptCTR(msg, ciphertext, lenmsg, this->key, ctr, ctrBitLen);
}

IppStatus AES_Crypt::decryptMessage(const Ipp8u *ciphertext, Ipp8u *msg, int &lenmsg, Ipp8u *ctr, int ctrBitLen)
{
    if (ctr == nullptr)    // If ctr not passed use internal ctr
        return ippsAESDecryptCTR(ciphertext, msg, lenmsg, this->key, this->ctr, AES_CTR_SIZE * 8);
    else
        return ippsAESDecryptCTR(ciphertext, msg, lenmsg, this->key, ctr, ctrBitLen);
}

AES_Crypt::~AES_Crypt()
{
    // If key is set overwrite sensitive data
    if (this->key != nullptr)
    {
        int ctxSize;
        ippsAESGetSize(&ctxSize);
        ippsAESInit(nullptr, this->keyLen / 8, key, ctxSize);
        delete[](Ipp8u *) this->key;
        this->key    = nullptr;
        this->keyLen = 0;
    }

    delete[] this->ctr;
}

inline Ipp8u *AES_Crypt::rand8(int size)
{
    Ipp8u *pX = new Ipp8u[size];
    std::srand(std::time(nullptr));    // Seed with current time
    for (int n = 0; n < size; ++n)
        pX[n] = rand();
    return pX;
}

SMS4_Crypt::SMS4_Crypt(Ipp8u *pkey, size_t keyLen)
{
    IppStatus status = ippStsNoErr;
    int ctxSize      = 0;

    status = ippsSMS4GetSize(&ctxSize);
    if (status != ippStsNoErr)
        throw std::runtime_error(ippGetStatusString(status));
    this->key = (IppsSMS4Spec *)(new Ipp8u[ctxSize]);
    this->ctr = new Ipp8u[SMS4_CTR_SIZE];
    memset(this->ctr, 1, SMS4_CTR_SIZE);

    if (pkey == nullptr)
    {
        pkey = rand8(keyLen / 8);
    }

    status = ippsSMS4Init(pkey, keyLen / 8, key, ctxSize);
    if (status != ippStsNoErr)
        throw std::runtime_error(ippGetStatusString(status));

    this->keyLen = keyLen;
}

IppStatus SMS4_Crypt::setKey(const Ipp8u *key, size_t keyLen)
{
    return ippsSMS4SetKey(key, keyLen / 8, this->key);
}

IppStatus SMS4_Crypt::resetCtr(Ipp8u *ctr, int ctrBitLen)
{
    if (ctrBitLen > SMS4_CTR_SIZE)
        return ippStsErr;
    if (this->ctr)
        delete this->ctr;

    this->ctr = new Ipp8u[SMS4_CTR_SIZE];
    if (!ctr)
        memset(this->ctr, 1, SMS4_CTR_SIZE);
    else
        memcpy(this->ctr + SMS4_CTR_SIZE - (ctrBitLen / 8), ctr, ctrBitLen / 8);

    return ippStsNoErr;
}

IppStatus SMS4_Crypt::encryptMessage(const Ipp8u *msg, int lenmsg, Ipp8u *ciphertext, Ipp8u *ctr, int ctrBitLen)
{
    if (ctr == nullptr)
        return ippsSMS4EncryptCTR(msg, ciphertext, lenmsg, this->key, this->ctr, SMS4_CTR_SIZE * 8);
    else
        return ippsSMS4EncryptCTR(msg, ciphertext, lenmsg, this->key, ctr, ctrBitLen);
}

IppStatus SMS4_Crypt::decryptMessage(const Ipp8u *ciphertext, Ipp8u *msg, int &lenmsg, Ipp8u *ctr, int ctrBitLen)
{
    if (ctr == nullptr)
        return ippsSMS4DecryptCTR(ciphertext, msg, lenmsg, this->key, this->ctr, SMS4_CTR_SIZE * 8);
    else
        return ippsSMS4DecryptCTR(ciphertext, msg, lenmsg, this->key, ctr, ctrBitLen);
}

SMS4_Crypt::~SMS4_Crypt()
{
    if (this->key != nullptr)
    {
        int ctxSize;
        ippsSMS4GetSize(&ctxSize);
        ippsSMS4Init(nullptr, this->keyLen / 8, key, ctxSize);
        delete[](Ipp8u *) this->key;
        this->key    = nullptr;
        this->keyLen = 0;
    }

    delete[] this->ctr;
}

inline Ipp8u *SMS4_Crypt::rand8(int size)
{
    Ipp8u *pX = new Ipp8u[size];
    std::srand(std::time(nullptr));    // Seed with current time
    for (int n = 0; n < size; ++n)
        pX[n] = rand();
    return pX;
}