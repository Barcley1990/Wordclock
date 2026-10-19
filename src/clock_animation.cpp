#include "clock_animation.h"

#include <string.h>

namespace ClockAnimation {

const Option Options[] = {
    {Effect::None, "none", "None (default)"},
    {Effect::Fadeout, "fadeout", "Fadeout"},
    {Effect::Matrix, "matrix", "Matrix"},
    {Effect::LeftRight, "left-right", "Left-right"},
};
const uint8_t OptionCount = sizeof(Options) / sizeof(Options[0]);

const char* effectId(Effect effect) {
    for (const auto& option : Options) {
        if (option.effect == effect) return option.id;
    }
    return "none";
}

bool parseEffect(const char* id, Effect& effect) {
    if (id == nullptr) return false;
    for (const auto& option : Options) {
        if (strcmp(id, option.id) == 0) {
            effect = option.effect;
            return true;
        }
    }
    return false;
}

bool Frame::pixel(int x, int y) const {
    return x >= 0 && x < Columns && y >= 0 && y < Rows &&
           (rows[y] & (uint16_t(1) << x)) != 0;
}

bool Frame::operator==(const Frame& other) const {
    return minutes == other.minutes && sameTextAs(other);
}

bool Frame::sameTextAs(const Frame& other) const {
    for (uint8_t y = 0; y < Rows; ++y) {
        if (rows[y] != other.rows[y]) return false;
    }
    return true;
}

static uint32_t dim(uint32_t color, uint8_t level) {
    const uint32_t r = ((color >> 16) & 255) * level / 255;
    const uint32_t g = ((color >> 8) & 255) * level / 255;
    const uint32_t b = (color & 255) * level / 255;
    return (r << 16) | (g << 8) | b;
}

static void draw(const Frame& frame, uint32_t color, uint32_t* pixels,
                 int offsetX = 0) {
    for (uint8_t y = 0; y < Rows; ++y) {
        for (uint8_t x = 0; x < Columns; ++x) {
            pixels[y * Columns + x] = frame.pixel(x - offsetX, y) ? color : 0;
        }
    }
}

void Animator::setFrame(const Frame& frame, uint32_t now) {
    if (initialized_ && frame == target_) return;
    if (initialized_ && frame.sameTextAs(target_)) {
        // Minute dots update without starting or restarting a text transition.
        target_.minutes = frame.minutes;
        previous_.minutes = frame.minutes;
        dirty_ = true;
        return;
    }
    previous_ = target_;
    target_ = frame;
    active_ = initialized_ && effect_ != Effect::None;
    initialized_ = true;
    startedAt_ = now;
    dirty_ = true;
}

void Animator::setEffect(Effect effect) {
    if (effect_ == effect) return;
    effect_ = effect;
    // Changing the selection settles the current transition immediately.
    active_ = false;
    dirty_ = true;
}

void Animator::setColor(uint32_t color) {
    color_ = color;
    dirty_ = true;
}

uint32_t Animator::duration() const {
    return effect_ == Effect::Matrix ? 3200u : 1200u;
}

bool Animator::render(uint32_t now, uint32_t* pixels) {
    if (!initialized_ || pixels == nullptr) return false;
    const uint32_t elapsed = now - startedAt_; // Safe across millis() rollover.
    if (active_ && elapsed >= duration()) {
        active_ = false;
        dirty_ = true;
    }
    if (!dirty_ && (!active_ || uint32_t(now - renderedAt_) < 33u)) return false;
    renderedAt_ = now;
    dirty_ = false;

    const Frame* minuteFrame = &target_;
    uint32_t minuteColor = color_;
    if (!active_) {
        draw(target_, color_, pixels);
    } else {
        const uint32_t half = duration() / 2;
        const bool entering = elapsed >= half;
        const uint32_t phase = entering ? elapsed - half : elapsed;
        const Frame& frame = entering ? target_ : previous_;
        const uint8_t level = entering ? phase * 255 / half : (half - phase) * 255 / half;
        minuteFrame = &frame;
        minuteColor = dim(color_, level);

        switch (effect_) {
        case Effect::Fadeout:
            draw(frame, minuteColor, pixels);
            break;
        case Effect::LeftRight: {
            // Old time exits right, then new time enters from the left.
            const int distance = phase * Columns / half;
            draw(frame, color_, pixels, entering ? distance - Columns : distance);
            break;
        }
        case Effect::Matrix: {
            minuteColor = dim(0x00ff40, level);
            // Stagger columns and leave a dim trail above the falling glyphs.
            const int travel = phase * (Rows + 8) / half;
            for (uint8_t x = 0; x < Columns; ++x) {
                const int delay = (x * 7) % 5;
                int offset = travel - delay;
                if (offset < 0) offset = 0;
                if (entering) {
                    offset -= Rows;
                    if (offset > 0) offset = 0;
                }
                for (uint8_t y = 0; y < Rows; ++y) {
                    uint32_t color = 0;
                    if (frame.pixel(x, y - offset)) {
                        color = frame.pixel(x, y + 1 - offset) ? 0x00ff40 : 0x90ffb0;
                    } else if (offset != 0) {
                        for (uint8_t tail = 1; tail <= 3; ++tail) {
                            if (frame.pixel(x, y + tail - offset)) {
                                color = dim(0x00ff40, 100u / tail);
                                break;
                            }
                        }
                    }
                    pixels[y * Columns + x] = color;
                }
            }
            break;
        }
        default:
            draw(target_, color_, pixels);
            break;
        }
    }
    for (uint8_t i = 0; i < MinuteLeds; ++i) {
        pixels[Rows * Columns + i] = (minuteFrame->minutes & (1u << i)) ? minuteColor : 0;
    }
    return true;
}

} // namespace ClockAnimation
