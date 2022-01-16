#pragma once

#include <stdio.h>
#include <stdexcept>

#include <ipp.h>
#include <ippcp.h>

#define MAX_HASH_MSG_LEN 65534

class Hash_Coder
{
  public:
    Hash_Coder(IppHashAlgId id);
    IppStatus update(Ipp8u *msg, size_t lenmsg);
    IppStatus calcFileHash(FILE *fptr, Ipp8u *hashCode);
    IppStatus getHash(Ipp8u *code);
    ~Hash_Coder();

  private:
    IppsHashState *context = nullptr;
};