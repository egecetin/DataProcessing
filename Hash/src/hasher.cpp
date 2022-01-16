#include "hasher.h"

Hash_Coder::Hash_Coder(IppHashAlgId id)
{
    IppStatus status;
    int ctxSize;

    status = ippsHashGetSize(&ctxSize);
    if (status != ippStsNoErr)
        throw std::runtime_error(ippGetStatusString(status));

    this->context = (IppsHashState *)new Ipp8u[ctxSize];
    status        = ippsHashInit(this->context, id);
    if (status != ippStsNoErr)
        throw std::runtime_error(ippGetStatusString(status));
}

IppStatus Hash_Coder::update(Ipp8u *msg, size_t lenmsg)
{
    return ippsHashUpdate(msg, lenmsg, this->context);
}

IppStatus Hash_Coder::calcFileHash(FILE *fptr, Ipp8u *hashCode)
{
    IppStatus status = ippStsNoErr;
    size_t size       = 0;
    Ipp8u buf[MAX_HASH_MSG_LEN];
    while ((size = fread(buf, 1, MAX_HASH_MSG_LEN, fptr)))
    {
        status = this->update(buf, size);
        if (status)
            return status;
    }

    return this->getHash(hashCode);
}

IppStatus Hash_Coder::getHash(Ipp8u *code)
{
    return ippsHashFinal(code, this->context);
}

Hash_Coder::~Hash_Coder()
{
    delete[](Ipp8u *) this->context;
    this->context = nullptr;
}