#ifndef _ILAYOUT_H
#define _ILAYOUT_H

#include <Arduino.h>

enum class Terms {
    ESIST,
    UHR,
    VIRTEL,
    HALB,
    DREIVIRTEL,
    VOR,
    NACH,

    EIN,
    EINE,
    EINS,
    ZWEI,
    DREI,
    VIER,
    FUENF,
    SECHS,
    SIEBEN,
    ACHT,
    NEUN,
    ZEHN,
    ELF,
    ZWOELF,
    ZWANZIG
};

class ILayout
{
private:
   

public:
    ILayout(/* args */) = default;

    virtual ~ILayout() = default;

    virtual uint8_t getMatrixRows() = 0;

    virtual uint8_t getMatrixCols() = 0;

    virtual void setMatrixPixel(uint8_t col, uint8_t row, bool state) = 0;

    virtual bool getMatrixPixel(uint8_t col, uint8_t row) = 0;

    virtual void setMatrixBackground() {};

    virtual void setMatrixTerm(Terms terms) = 0;

    virtual void run() {};
};


#endif //_ILAYOUT_H