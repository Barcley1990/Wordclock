#ifndef _DEBOUNCE_H
#define _DEBOUNCE_H

#include <Arduino.h>

typedef enum DebounceAlgorithm{
    COUNT_DOWN = 0,
    COUNT_UP_DOWN = 1
}DebounceType;

class Debounce
{
private:
    uint16_t _debounceCnt = 0u;
    uint16_t _lastTime = 0u;
    uint8_t _pin;
    uint16_t _time;
    uint8_t _period;
    void(*_callback)(void) = nullptr;

    uint16_t resetCounter();
    

public:
    Debounce(uint8_t pin, uint16_t time,  uint8_t period, void(*callback)(void));
    ~Debounce();
    void poll();
};

/**
 * @brief Construct a new Debounce:: Debounce object
 * 
 * @param pin       Pin to poll
 * @param time      Time in milliseconds
 * @param period    Poll period in milliseconds
 * @param callback  Callback function
 */
Debounce::Debounce(uint8_t pin, uint16_t time, uint8_t period, void(*callback)(void))
{
    _callback = callback;
    _pin = pin;
    _time = time;
    _period = period;
    _debounceCnt = resetCounter();
}

/**
 * @brief Destroy the Debounce:: Debounce object
 * 
 */
Debounce::~Debounce()
{
}

/**
 * @brief Poll pin. Execute callback after debounce time passed.
 * 
 */
void Debounce::poll()
{
    _pin == HIGH ? _debounceCnt-- : _debounceCnt = 0u;

    if(_debounceCnt == 0u)
    {
        _debounceCnt = resetCounter();
        if(_callback != nullptr)
            _callback();
    }
}

/**
 * @brief Reset Debounce Counter
 * 
 * @return uint16_t 
 */
uint16_t Debounce::resetCounter()
{
    return (uint16_t)(_time/_period);
}


#endif //_DEFOUNCE_H