#include "compression.h"

LZSS_Comp::LZSS_Comp()
{
    int ctxSize = 0;

    ippsLZSSGetSize_8u(&ctxSize);
    this->context = (IppLZSSState_8u *)new Ipp8u[ctxSize];
}

IppStatus LZSS_Comp::encode(char *pathSrc, char *pathDest)
{
    IppStatus status = ippStsNoErr;

    FILE *src  = fopen(pathSrc, "rb");
    FILE *dest = fopen(pathDest, "wb");

    if (!(src && dest))
    {
        fclose(src);
        fclose(dest);
        std::filesystem::exists(pathDest) ? std::filesystem::remove(pathDest) : void();
        return ippStsNoOperation;
    }
    status = this->encode(src, dest);

    fclose(src);
    fclose(dest);

    return status;
}

IppStatus LZSS_Comp::decode(char *pathSrc, char *pathDest)
{
    IppStatus status = ippStsNoErr;

    FILE *src  = fopen(pathSrc, "rb");
    FILE *dest = fopen(pathDest, "wb");

    if (!(src && dest))
    {
        fclose(src);
        fclose(dest);
        std::filesystem::exists(pathDest) ? std::filesystem::remove(pathDest) : void();
        return ippStsNoOperation;
    }

    status = this->decode(src, dest);

    fclose(src);
    fclose(dest);

    return status;
}

IppStatus LZSS_Comp::encode(FILE *fsrc, FILE *fdst)
{
    IppStatus status = ippStsNoErr;

    if (status = ippsEncodeLZSSInit_8u(this->context))
        return status;

    int size_buff, size_out;
    Ipp8u *buff_org = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ), *buff = buff_org;
    Ipp8u *out_org = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ + COMP_EXTEND), *out = out_org;

    size_out  = COMP_BUFSIZ + COMP_EXTEND;
    size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc);
    if (size_buff)
    {
        while (true)
        {
            status = ippsEncodeLZSS_8u(&buff, &size_buff, &out, &size_out, this->context);
            if (status == ippStsDstSizeLessExpected)
            {
                fwrite(out_org, COMP_BUFSIZ + COMP_EXTEND - size_out, 1, fdst);
                out      = out_org;
                size_out = COMP_BUFSIZ + COMP_EXTEND;
            }
            else if (status == ippStsNoErr)
            {
                fwrite(out_org, COMP_BUFSIZ + COMP_EXTEND - size_out, 1, fdst);
                size_buff = 0;
                size_out  = COMP_BUFSIZ + COMP_EXTEND;
                buff      = buff_org;
                out       = out_org;
                size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc);
                if (!size_buff)
                    break;
            }
            else
                break;
        }
    }

    if (!status)
    {    // Last bits
        status = ippsEncodeLZSSFlush_8u(&out, &size_out, this->context);
        fwrite(out_org, COMP_BUFSIZ + COMP_EXTEND - size_out, 1, fdst);
    }

    free(buff_org);
    free(out_org);

    return status;
}

IppStatus LZSS_Comp::decode(FILE *fsrc, FILE *fdst)
{
    IppStatus status = ippStsNoErr;

    if (status = ippsDecodeLZSSInit_8u(this->context))
        return status;

    int size_buff, size_out;
    Ipp8u *buff_org = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ), *buff = buff_org;
    Ipp8u *out_org = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ * 4), *out = out_org;

    size_out  = COMP_BUFSIZ * 4;
    size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc);
    if (size_buff)
    {
        while (true)
        {
            status = ippsDecodeLZSS_8u(&buff, &size_buff, &out, &size_out, this->context);
            if (status == ippStsDstSizeLessExpected)
            {
                fwrite(out_org, COMP_BUFSIZ * 4 - size_out, 1, fdst);
                out      = out_org;
                size_out = COMP_BUFSIZ * 4;
            }
            else if (status == ippStsNoErr)
            {
                fwrite(out_org, COMP_BUFSIZ * 4 - size_out, 1, fdst);
                size_buff = COMP_BUFSIZ;
                size_out  = COMP_BUFSIZ * 4;
                buff      = buff_org;
                out       = out_org;
                size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc);
                if (!size_buff)
                    break;
            }
            else
                break;
        }
    }

    free(buff_org);
    free(out_org);

    return status;
}

LZSS_Comp::~LZSS_Comp()
{
    delete[](Ipp8u *) this->context;
}

LZO_Comp::LZO_Comp(COMPRESSION_METHOD id)
{
    Ipp32u ctxSize = 0;

    switch (id)
    {
        case LZO_FAST:
            ippsEncodeLZOGetSize(IppLZO1X1ST, 0, &ctxSize);
            this->context = (IppLZOState_8u *)new Ipp8u[ctxSize];
            ippsEncodeLZOInit_8u(IppLZO1X1ST, 0, this->context);
            break;
        case LZO_SLOW:
            ippsEncodeLZOGetSize(IppLZO1XST, 0, &ctxSize);
            this->context = (IppLZOState_8u *)new Ipp8u[ctxSize];
            ippsEncodeLZOInit_8u(IppLZO1X1ST, 0, this->context);
            break;
    }
}

IppStatus LZO_Comp::encode(char *pathSrc, char *pathDest)
{
    IppStatus status = ippStsNoErr;

    FILE *src  = fopen(pathSrc, "rb");
    FILE *dest = fopen(pathDest, "wb");

    if (!(src && dest))
    {
        fclose(src);
        fclose(dest);
        std::filesystem::exists(pathDest) ? std::filesystem::remove(pathDest) : void();
        return ippStsNoOperation;
    }

    status = this->encode(src, dest);

    fclose(src);
    fclose(dest);

    return status;
}

IppStatus LZO_Comp::decode(char *pathSrc, char *pathDest)
{
    IppStatus status = ippStsNoErr;

    FILE *src  = fopen(pathSrc, "rb");
    FILE *dest = fopen(pathDest, "wb");

    if (!(src && dest))
    {
        fclose(src);
        fclose(dest);
        std::filesystem::exists(pathDest) ? std::filesystem::remove(pathDest) : void();
        return ippStsNoOperation;
    }
    status = this->decode(src, dest);

    fclose(src);
    fclose(dest);

    return status;
}

IppStatus LZO_Comp::encode(FILE *fsrc, FILE *fdst)
{
    IppStatus status = ippStsNoErr;
    Ipp32u size_buff, size_out;

    if (!this->context)
        return ippStsNoOperation;

    Ipp8u *buff = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ);
    Ipp8u *out  = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ + COMP_EXTEND);

    size_buff = COMP_BUFSIZ, size_out = COMP_BUFSIZ + COMP_EXTEND;
    while (size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc))
    {
        if (status = ippsEncodeLZO_8u(buff, size_buff, out, &size_out, this->context))
            break;
        fwrite(out, size_out, 1, fdst);
        size_out = COMP_BUFSIZ + COMP_EXTEND;
    }

    free(buff);
    free(out);

    return status;
}

IppStatus LZO_Comp::decode(FILE *fsrc, FILE *fdst)
{
    IppStatus status = ippStsNoErr;
    Ipp32u size_buff, size_out;
    Ipp8u *buff = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ);
    Ipp8u *out  = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ * 4);

    size_buff = COMP_BUFSIZ, size_out = COMP_BUFSIZ * 4;

    size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc);
    if (size_buff)
    {
        while (true)
        {
            status = ippsDecodeLZOSafe_8u(buff, size_buff, out, &size_out);
            if (status == ippStsDstSizeLessExpected)
            {
                out = (Ipp8u *)realloc(out, sizeof(Ipp8u) * size_out * 2);
                size_out *= 2;
            }
            else if (status == ippStsNoErr)
            {
                fwrite(out, size_out, 1, fdst);
                size_out  = COMP_BUFSIZ * 4;
                size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc);
                if (!size_buff)
                    break;
            }
            else
                break;
        }
    }

    free(buff);
    free(out);

    return status;
}

LZO_Comp::~LZO_Comp()
{
    delete[](Ipp8u *) this->context;
}

LZ4_Comp::LZ4_Comp(COMPRESSION_METHOD id)
{
    int ctxSize;

    switch (id)
    {
        case LZ4_HC:
            // Reserved
        default:
            ippsEncodeLZ4HashTableGetSize_8u(&ctxSize);
            this->hashTable = new Ipp8u[ctxSize];
            ippsEncodeLZ4HashTableInit_8u(this->hashTable, ctxSize);
    }
}

IppStatus LZ4_Comp::encode(char *pathSrc, char *pathDest)
{
    IppStatus status = ippStsNoErr;
    FILE *src         = fopen(pathSrc, "rb");
    FILE *dest        = fopen(pathDest, "wb");

    if (!(src && dest))
    {
        fclose(src);
        fclose(dest);
        std::filesystem::exists(pathDest) ? std::filesystem::remove(pathDest) : void();
        return ippStsNoOperation;
    }

    status = this->encode(src, dest);

    fclose(src);
    fclose(dest);

    return status;
}

IppStatus LZ4_Comp::decode(char *pathSrc, char *pathDest)
{
    IppStatus status = ippStsNoErr;

    FILE *src  = fopen(pathSrc, "rb");
    FILE *dest = fopen(pathDest, "wb");

    if (!(src && dest))
    {
        fclose(src);
        fclose(dest);
        std::filesystem::exists(pathDest) ? std::filesystem::remove(pathDest) : void();
        return ippStsNoOperation;
    }

    status = this->decode(src, dest);

    fclose(src);
    fclose(dest);

    return status;
}

IppStatus LZ4_Comp::encode(FILE *fsrc, FILE *fdst)
{
    IppStatus status = ippStsNoErr;
    int size_buff, size_out;
    Ipp8u *buff = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ);
    Ipp8u *out  = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ + COMP_EXTEND);

    size_buff = COMP_BUFSIZ, size_out = COMP_BUFSIZ + COMP_EXTEND;
    while (size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc))
    {
        if (status = ippsEncodeLZ4_8u(buff, size_buff, out, &size_out, this->hashTable))
            break;
        fwrite(out, size_out, 1, fdst);
        size_out = COMP_BUFSIZ + COMP_EXTEND;
    }

    free(buff);
    free(out);

    return status;
}

IppStatus LZ4_Comp::decode(FILE *fsrc, FILE *fdst)
{
    IppStatus status = ippStsNoErr;
    int size_buff, size_out;
    Ipp8u *buff = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ);
    Ipp8u *out  = (Ipp8u *)malloc(sizeof(Ipp8u) * COMP_BUFSIZ * 4);

    size_out  = COMP_BUFSIZ * 4;
    size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc);
    if (size_buff)
    {
        while (true)
        {
            status = ippsDecodeLZ4_8u(buff, size_buff, out, &size_out);
            if (status == ippStsDstSizeLessExpected)
            {
                out = (Ipp8u *)realloc(out, sizeof(Ipp8u) * size_out * 2);
                size_out *= 2;
            }
            else if (status == ippStsNoErr)
            {
                fwrite(out, size_out, 1, fdst);
                size_out  = COMP_BUFSIZ * 4;
                size_buff = fread(buff, 1, COMP_BUFSIZ, fsrc);
                if (!size_buff)
                    break;
            }
            else
                break;
        }
    }

    free(buff);
    free(out);

    return status;
}

LZ4_Comp::~LZ4_Comp()
{
    delete[](Ipp8u *) this->hashTable;
}
