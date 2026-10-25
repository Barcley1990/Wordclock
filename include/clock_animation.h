#pragma once

#include <stdint.h>

namespace ClockAnimation {

constexpr uint8_t Columns = 11;
constexpr uint8_t Rows = 10;
constexpr uint8_t MinuteLeds = 4;
constexpr uint16_t PixelCount = Columns * Rows + MinuteLeds;

enum class Effect : uint8_t { None, Fadeout, Matrix, LeftRight, Noise, Random };

struct Option {
    Effect effect;
    const char* id;
    const char* label;
};

// Stable IDs are used by both persistent settings and the web API.
extern const Option Options[];
extern const uint8_t OptionCount;
const char* effectId(Effect effect);
bool parseEffect(const char* id, Effect& effect);

struct Frame {
    uint16_t rows[Rows] = {};
    uint8_t minutes = 0;

    bool pixel(int x, int y) const;
    bool sameTextAs(const Frame& other) const;
    bool operator==(const Frame& other) const;
};

// No delays, heap allocations or hardware dependencies. Pixels are logical,
// row-major RGB values followed by the four minute LEDs.
class Animator {
public:
    void setFrame(const Frame& frame, uint32_t now);
    void setEffect(Effect effect);
    void preview(Effect effect, uint32_t now);
    void setColor(uint32_t color);
    void requestRedraw() { dirty_ = true; }
    bool render(uint32_t now, uint32_t* pixels);
    bool active() const { return active_; }

private:
    Frame previous_;
    Frame target_;
    Effect effect_ = Effect::None;
    Effect playingEffect_ = Effect::None;
    uint32_t color_ = 0;
    uint32_t startedAt_ = 0;
    uint32_t renderedAt_ = 0;
    uint32_t noiseState_ = 1;
    uint32_t randomState_ = 0x9e3779b9u;
    uint8_t noiseRanks_[Rows * Columns] = {};
    uint8_t noiseLetterCount_ = 0;
    bool initialized_ = false;
    bool active_ = false;
    bool dirty_ = true;

    uint32_t duration() const;
    void prepareNoise(uint32_t now);
    Effect resolveEffect(Effect effect, uint32_t now);
};

} // namespace ClockAnimation
