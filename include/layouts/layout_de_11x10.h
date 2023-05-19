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

#define TERMS_DE_NUM (21u)

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

    void setMatrixPixel(uint8_t col, uint8_t row, State state) override
    {
        if((col>COLUMNS) || (row>ROWS)) return;
        switch (state) {
        case SET: _matrixBuffer[row] |= (1<<(COLUMNS-col)); break;
        case CLEAR: _matrixBuffer[row] &= ~(1<<(COLUMNS-col)); break;
        default: /*should never be called*/ break;
        }
    }

    State getMatrixPixel(uint8_t col, uint8_t row)
    {
        return (State)((_matrixBuffer[row] >> (COLUMNS-col)) & 0x1);
    }

    void setMatrixTime(Terms term) override
    {
        switch (term)
        {
        case Terms::ESIST:
            setMatrixPixel(0,0,SET);
            setMatrixPixel(0,1,SET);
            setMatrixPixel(0,3,SET);
            setMatrixPixel(0,4,SET);
            setMatrixPixel(0,5,SET);
            break;
        case Terms::UHR:
            setMatrixPixel(9,8,SET);
            setMatrixPixel(9,9,SET);
            setMatrixPixel(9,10,SET);
            break;
        
        default:
            break;
        }
    }




private:
    uint8_t _layoutRows;
    uint8_t _layoutCols;
    // max 16 collumns!
    uint16_t _matrixBuffer[ROWS];
    
};



#endif //_LAYOUT_DE_11X10_H