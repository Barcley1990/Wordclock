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
    DREI_1,
    DREI_2,
    VIER_1,
    VIER_2,
    FUENF_1,
    FUENF_2,
    SECHS,
    SIEBEN,
    ACHT,
    NEUN,
    ZEHN_1,
    ZEHN_2,
    ELF,
    ZWOELF,
    ZWANZIG
};

class ILayout
{
private:


public:
    /**
     * @brief Construct a new ILayout.
     */
    ILayout(/* args */) = default;

    /**
     * @brief Destroy the ILayout.
     */
    virtual ~ILayout() = default;

    /**
     * @brief Get matrix row count.
     * @return Number of rows.
     */
    virtual uint8_t getMatrixRows() = 0;

    /**
     * @brief Get matrix column count.
     * @return Number of columns.
     */
    virtual uint8_t getMatrixCols() = 0;

    /**
     * @brief Clear the matrix buffer.
     */
    virtual void clearMatrix() = 0;

    /**
     * @brief Set a matrix pixel.
     * @param col Column index.
     * @param row Row index.
     * @param state Pixel state.
     */
    virtual void setMatrixPixel(uint8_t col, uint8_t row, bool state) = 0;

    /**
     * @brief Get a matrix pixel.
     * @param col Column index.
     * @param row Row index.
     * @return Pixel state.
     */
    virtual bool getMatrixPixel(uint8_t col, uint8_t row) = 0;

    /**
     * @brief Set background pixels if supported.
     */
    virtual void setMatrixBackground() {};

    /**
     * @brief Set a term in the matrix.
     * @param terms Term to display.
     */
    virtual void setMatrixTerm(Terms terms) = 0;

    /**
     * @brief Optional layout update tick.
     */
    virtual void run() {};
};


#endif //_ILAYOUT_H
