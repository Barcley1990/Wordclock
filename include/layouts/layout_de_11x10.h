#ifndef _LAYOUT_DE_11X10_H
#define _LAYOUT_DE_11X10_H

/*************************************************************
E S K I S T L F Ü N F    ==> ES IST FÜNF
Z E H N Z W A N Z I G    ==> ZEHN ZWANZIG
D R E I V I E R T E L    ==> DREI|VIERTEL
T G N A C H V O R J M    ==> NACH VOR
H A L B Q Z W Ö L F P    ==> HALB ZWÖLF
Z W E I N S I E B E N    ==> ZW|EI|N|S|IEBEN
K D R E I R H F Ü N F    ==> DREI FÜNF
E L F N E U N V I E R    ==> ELF NEUN VIER
W A C H T Z E H N R S    ==> ACHT ZEHN
B S E C H S F M U H R    ==> SECHS UHR
**************************************************************/
#include "ilayout.h"

#define ROWS                    (10u)
#define COLUMNS                 (11u)
#define MINUTE_LEDS             (4u)
#define NUM_LEDS                ((COLUMNS * ROWS) + MINUTE_LEDS)

// Interface Implementor
class Layout_De_11x10 : public ILayout
{
public:
    Layout_De_11x10() : ILayout()
    {
        _layoutRows = ROWS;
        _layoutCols = COLUMNS;
    }
    ~Layout_De_11x10() {}

    uint8_t getMatrixRows() override {return _layoutRows;}

    uint8_t getMatrixCols() override {return _layoutCols;}

    void clearMatrix() {std::fill_n(_matrixBuffer, ROWS, 0u);}

    void setMatrixPixel(uint8_t col, uint8_t row, bool state) override
    {
        if((col>COLUMNS) || (row>ROWS)) return;
        if (state == true) {
            _matrixBuffer[row] |= (1<<(COLUMNS-col));
        }else{
             _matrixBuffer[row] &= ~(1<<(COLUMNS-col));
        }
    }

    bool getMatrixPixel(uint8_t col, uint8_t row)
    {
        return ((_matrixBuffer[row] >> (COLUMNS-col)) & 0x1);
    }

    bool getMatrixPixel(uint16_t i)
    {
        return 0;
    }

    void setMatrixTerm(Terms term) override
    {
        switch (term)
        {
        case Terms::ESIST:
            setMatrixPixel(0,0,true);
            setMatrixPixel(1,0,true);
            setMatrixPixel(3,0,true);
            setMatrixPixel(4,0,true);
            setMatrixPixel(5,0,true);
            //Serial.print("ES IST ");
            break;
        case Terms::UHR:
            setMatrixPixel(8,9,true);
            setMatrixPixel(9,9,true);
            setMatrixPixel(10,9,true);
            //Serial.print("UHR ");
            break;

        case Terms::VIRTEL:
            setMatrixPixel(4,2,true);
            setMatrixPixel(5,2,true);
            setMatrixPixel(6,2,true);
            setMatrixPixel(7,2,true);
            setMatrixPixel(8,2,true);
            setMatrixPixel(9,2,true);
            setMatrixPixel(10,2,true);
            //Serial.print("VIRTEL ");
        break;

        case Terms::HALB:
            setMatrixPixel(0,4,true);
            setMatrixPixel(1,4,true);
            setMatrixPixel(2,4,true);
            setMatrixPixel(3,4,true);
            //Serial.print("HALB ");
        break;

        case Terms::DREIVIRTEL:
            setMatrixPixel(0,2,true);
            setMatrixPixel(1,2,true);
            setMatrixPixel(2,2,true);
            setMatrixPixel(3,2,true);
            setMatrixPixel(5,2,true);
            setMatrixPixel(6,2,true);
            setMatrixPixel(7,2,true);
            setMatrixPixel(8,2,true);
            setMatrixPixel(9,2,true);
            setMatrixPixel(10,2,true);
            //Serial.print("DREIVIRTEL ");
        break;

        case Terms::VOR:
            setMatrixPixel(6,3,true);
            setMatrixPixel(7,3,true);
            setMatrixPixel(8,3,true);
            //Serial.print("VOR ");
        break;

        case Terms::NACH:
            setMatrixPixel(2,3,true);
            setMatrixPixel(3,3,true);
            setMatrixPixel(4,3,true);
            setMatrixPixel(5,3,true);
            //Serial.print("NACH ");
        break;

        case Terms::EIN:
            setMatrixPixel(2,5,true);
            setMatrixPixel(3,5,true);
            setMatrixPixel(4,5,true);
            //Serial.print("EIN ");
        break;

        case Terms::EINS:
            setMatrixPixel(2,5,true);
            setMatrixPixel(3,5,true);
            setMatrixPixel(4,5,true);
            //Serial.print("EINS ");
        break;

        case Terms::ZWEI:
            setMatrixPixel(0,5,true);
            setMatrixPixel(1,5,true);
            setMatrixPixel(2,5,true);
            setMatrixPixel(3,5,true);
            //Serial.print("ZWEI ");
        break;

        case Terms::DREI_1:
            setMatrixPixel(0,2,true);
            setMatrixPixel(1,2,true);
            setMatrixPixel(2,2,true);
            setMatrixPixel(3,2,true);
            //Serial.print("DREI ");
        break;

        case Terms::DREI_2:
            setMatrixPixel(1,6,true);
            setMatrixPixel(2,6,true);
            setMatrixPixel(3,6,true);
            setMatrixPixel(4,6,true);
            //Serial.print("DREI ");
        break;

        case Terms::VIER_1:
            setMatrixPixel(4,2,true);
            setMatrixPixel(5,2,true);
            setMatrixPixel(6,2,true);
            setMatrixPixel(7,2,true);
            ///Serial.print("VIER ");
        break;

        case Terms::VIER_2:
            setMatrixPixel(7,7,true);
            setMatrixPixel(8,7,true);
            setMatrixPixel(9,7,true);
            setMatrixPixel(10,7,true);
            ///Serial.print("VIER ");
        break;

        case Terms::FUENF_1:
            setMatrixPixel(7,0,true);
            setMatrixPixel(8,0,true);
            setMatrixPixel(9,0,true);
            setMatrixPixel(10,0,true);
            //Serial.print("FÜNF ");
        break;

        case Terms::FUENF_2:
            setMatrixPixel(7,6,true);
            setMatrixPixel(8,6,true);
            setMatrixPixel(9,6,true);
            setMatrixPixel(10,6,true);
            //Serial.print("FÜNF ");
        break;

        case Terms::SECHS:
            setMatrixPixel(1,9,true);
            setMatrixPixel(2,9,true);
            setMatrixPixel(3,9,true);
            setMatrixPixel(4,9,true);
            setMatrixPixel(5,9,true);
            //Serial.print("SECHS ");
        break;

        case Terms::SIEBEN:
            setMatrixPixel(5,5,true);
            setMatrixPixel(6,5,true);
            setMatrixPixel(7,5,true);
            setMatrixPixel(8,5,true);
            setMatrixPixel(9,5,true);
            setMatrixPixel(10,5,true);
            //Serial.print("SIEBEN ");
        break;

        case Terms::ACHT:
            setMatrixPixel(1,8,true);
            setMatrixPixel(2,8,true);
            setMatrixPixel(3,8,true);
            setMatrixPixel(4,8,true);
            //Serial.print("ACHT ");
        break;

        case Terms::NEUN:
            setMatrixPixel(3,7,true);
            setMatrixPixel(4,7,true);
            setMatrixPixel(5,7,true);
            setMatrixPixel(6,7,true);
            //Serial.print("NEUN ");
        break;

        case Terms::ZEHN:
            setMatrixPixel(0,1,true);
            setMatrixPixel(1,1,true);
            setMatrixPixel(2,1,true);
            setMatrixPixel(3,1,true);
            //Serial.print("ZEHN ");
        break;

        case Terms::ELF:
            setMatrixPixel(0,7,true);
            setMatrixPixel(1,7,true);
            setMatrixPixel(2,7,true);
            //Serial.print("ELF ");
        break;

        case Terms::ZWOELF:
            setMatrixPixel(5,4,true);
            setMatrixPixel(6,4,true);
            setMatrixPixel(7,4,true);
            setMatrixPixel(8,4,true);
            setMatrixPixel(9,4,true);
            //Serial.print("ZWÖLF ");
        break;

        case Terms::ZWANZIG:
            setMatrixPixel(4,1,true);
            setMatrixPixel(5,1,true);
            setMatrixPixel(6,1,true);
            setMatrixPixel(7,1,true);
            setMatrixPixel(8,1,true);
            setMatrixPixel(9,1,true);
            setMatrixPixel(10,1,true);
            //Serial.print("ZWANZIG ");
        break;

        default:
            //Serial.print("Error! ");
            break;
        }
        //Serial.println("");
    }




private:
    uint8_t _layoutRows;
    uint8_t _layoutCols;
    // max 16 collumns!
    uint16_t _matrixBuffer[ROWS] = {0};

};



#endif //_LAYOUT_DE_11X10_H