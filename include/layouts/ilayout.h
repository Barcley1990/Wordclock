#ifndef _ILAYOUT_H
#define _ILAYOUT_H

#include <Arduino.h>

enum Terms {
    ESIST,
    UHR,
    VIRTEL,
    HALB,
    DREIVIRTEL,
    VOR,
    NACH,

    EIN,
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

enum State {
    SET = 1,
    CLEAR = 0
};

class ILayout
{
private:
   

public:
    ILayout(/* args */) = default;

    virtual ~ILayout() = default;

    virtual uint8_t getMatrixRows() = 0;

    virtual uint8_t getMatrixCols() = 0;

    virtual void setMatrixPixel(uint8_t col, uint8_t row, State state) = 0;

    virtual State getMatrixPixel(uint8_t col, uint8_t row) = 0;

    virtual void setMatrixBackground() {};

    virtual void setMatrixTime(Terms terms) = 0;

    virtual void run() {};
};


#endif //_ILAYOUT_H