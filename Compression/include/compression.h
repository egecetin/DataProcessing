#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <filesystem>

#include <ipp.h>
#include <ippcp.h>

#define COMP_BUFSIZ 131072    // 128 kB
#define COMP_EXTEND 1024      //   1 kB

enum COMPRESSION_METHOD
{
    NO_COMPRESS,
    LZSS,            // Lempel-Ziv-Storer-Szymansk
    ZLIB_FAST,       // (Reserved)
    ZLIB_AVERAGE,    // (Reserved)
    ZLIB_SLOW,       // (Reserved)
    LZO_FAST,        // Lempel-Ziv-Oberhumer (IppLZO1X1ST)
    LZO_SLOW,        // Lempel-Ziv-Oberhumer (IppLZO1XST)
    LZ4,
    LZ4_HC,    // High-compression mode (Reserved)
    COMPRESSION_MAX
};

class LZSS_Comp
{
  public:
    LZSS_Comp();
    IppStatus encode(char *pathSrc, char *pathDest);
    IppStatus decode(char *pathSrc, char *pathDest);
    IppStatus encode(FILE *fsrc, FILE *fdst);
    IppStatus decode(FILE *fsrc, FILE *fdst);
    ~LZSS_Comp();

  private:
    IppLZSSState_8u *context = nullptr;
};

class LZO_Comp
{
  public:
    LZO_Comp(COMPRESSION_METHOD id);
    IppStatus encode(char *pathSrc, char *pathDest);
    IppStatus decode(char *pathSrc, char *pathDest);
    IppStatus encode(FILE *fsrc, FILE *fdst);
    IppStatus decode(FILE *fsrc, FILE *fdst);
    ~LZO_Comp();

  private:
    IppLZOState_8u *context = nullptr;
};

class LZ4_Comp
{
  public:
    LZ4_Comp(COMPRESSION_METHOD id);
    IppStatus encode(char *pathSrc, char *pathDest);
    IppStatus decode(char *pathSrc, char *pathDest);
    IppStatus encode(FILE *fsrc, FILE *fdst);
    IppStatus decode(FILE *fsrc, FILE *fdst);
    ~LZ4_Comp();

  private:
    Ipp8u *hashTable = nullptr;
    Ipp8u *dict      = nullptr;    // Reserved
};