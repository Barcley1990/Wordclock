#include "clock_animation.h"

#include <string.h>

namespace ClockAnimation {

const Option Options[] = {
    {Effect::None, "none", "None (default)"},
    {Effect::Fadeout, "fadeout", "Fadeout"},
    {Effect::Matrix, "matrix", "Matrix"},
    {Effect::LeftRight, "left-right", "Left-right"},
    {Effect::Noise, "noise", "Noise"},
    {Effect::Random, "random", "Random"},
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

static uint32_t blend(uint32_t from, uint32_t to, uint8_t level) {
    uint32_t color = 0;
    for (uint8_t shift = 0; shift <= 16; shift += 8) {
        const uint32_t channel = (((from >> shift) & 255u) * (255u - level) +
                                  ((to >> shift) & 255u) * level) / 255u;
        color |= channel << shift;
    }
    return color;
}

static void draw(const Frame& frame, uint32_t color, uint32_t* pixels,
                 int alternatingOffsetX = 0) {
    uint8_t occupiedRow = 0;
    for (uint8_t y = 0; y < Rows; ++y) {
        const int offsetX = (occupiedRow & 1u) ? -alternatingOffsetX : alternatingOffsetX;
        for (uint8_t x = 0; x < Columns; ++x) {
            pixels[y * Columns + x] = frame.pixel(x - offsetX, y) ? color : 0;
        }
        if (frame.rows[y] != 0) ++occupiedRow;
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
    playingEffect_ = resolveEffect(effect_, now);
    initialized_ = true;
    startedAt_ = now;
    prepareNoise(now);
    prepareMatrix(now);
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

void Animator::preview(Effect effect, uint32_t now) {
    if (!initialized_) return;
    previous_ = target_;
    playingEffect_ = resolveEffect(effect, now);
    startedAt_ = now;
    prepareNoise(now);
    prepareMatrix(now);
    active_ = effect != Effect::None;
    dirty_ = true;
}

static uint32_t nextRandom(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

void Animator::prepareMatrix(uint32_t now) {
    if (playingEffect_ != Effect::Matrix) return;
    uint32_t state = (now ^ 0xa341316cu) | 1u;
    for (uint8_t x = 0; x < Columns; ++x) {
        matrixStep_[x] = 55u + nextRandom(state) % 46u;
        matrixOffset_[x] = nextRandom(state) % (Rows + 7u);
        matrixRevealDelay_[x] = nextRandom(state) % 181u;
    }
}

Effect Animator::resolveEffect(Effect effect, uint32_t now) {
    if (effect != Effect::Random) return effect;
    uint8_t count = 0;
    for (const auto& option : Options) {
        if (option.effect != Effect::None && option.effect != Effect::Random) ++count;
    }
    randomState_ ^= now;
    if (!randomState_) randomState_ = 1;
    uint8_t selected = nextRandom(randomState_) % count;
    for (const auto& option : Options) {
        if (option.effect == Effect::None || option.effect == Effect::Random) continue;
        if (selected-- == 0) return option.effect;
    }
    return Effect::None;
}

void Animator::prepareNoise(uint32_t now) {
    if (playingEffect_ != Effect::Noise) return;
    noiseState_ = now | 1u;
    uint32_t state = noiseState_;
    uint8_t letters[Rows * Columns];
    noiseLetterCount_ = 0;
    memset(noiseRanks_, 255, sizeof(noiseRanks_));
    for (uint8_t i = 0; i < Rows * Columns; ++i) {
        if (target_.pixel(i % Columns, i / Columns)) letters[noiseLetterCount_++] = i;
    }
    // Shuffle once per transition: settled letters keep their random reveal order.
    for (uint8_t remaining = noiseLetterCount_; remaining > 1; --remaining) {
        const uint8_t selected = nextRandom(state) % remaining;
        const uint8_t tmp = letters[remaining - 1];
        letters[remaining - 1] = letters[selected];
        letters[selected] = tmp;
    }
    for (uint8_t rank = 0; rank < noiseLetterCount_; ++rank) {
        noiseRanks_[letters[rank]] = rank;
    }
}

void Animator::renderMatrix(uint32_t elapsed, uint32_t* pixels) const {
    constexpr uint32_t green = 0x00ff00;
    constexpr uint32_t oldFadeEnd = 300u;
    constexpr uint32_t rainFadeEnd = 600u;
    constexpr uint32_t revealStart = 1600u; // One full second of rain after fading in.
    constexpr uint32_t revealEnd = 2800u;
    constexpr uint32_t rainEnd = 3000u;
    const Frame* minuteFrame = &target_;
    uint32_t minuteColor = 0;

    if (elapsed < oldFadeEnd) {
        const uint32_t oldColor = dim(color_, (oldFadeEnd - elapsed) * 255u / oldFadeEnd);
        draw(previous_, oldColor, pixels);
        minuteFrame = &previous_;
        minuteColor = oldColor;
    } else if (elapsed >= rainEnd) {
        const uint32_t finalColor = blend(green, color_, (elapsed - rainEnd) * 255u / 200u);
        draw(target_, finalColor, pixels);
        minuteColor = finalColor;
    } else {
        const uint8_t rainLevel = elapsed < rainFadeEnd ?
            (elapsed - oldFadeEnd) * 255u / (rainFadeEnd - oldFadeEnd) :
            elapsed >= revealEnd ? (rainEnd - elapsed) * 255u / (rainEnd - revealEnd) : 255u;
        // Freeze the last rain frame while it fades, keeping the emerging text still.
        const uint32_t rainTime = (elapsed < revealEnd ? elapsed : revealEnd) - oldFadeEnd;
        for (uint8_t x = 0; x < Columns; ++x) {
            const int head = (rainTime / matrixStep_[x] + matrixOffset_[x]) % (Rows + 7u);
            for (uint8_t y = 0; y < Rows; ++y) {
                const uint32_t settleAt = revealStart + matrixRevealDelay_[x] + y * 110u;
                const bool settled = target_.pixel(x, y) && elapsed >= settleAt;
                uint32_t color = 0;
                if (settled) color = green;
                else {
                    uint8_t brightness = 0;
                    // Two staggered streams per column roughly double the code density.
                    for (uint8_t stream = 0; stream < 2; ++stream) {
                        const int streamHead = (head + stream * ((Rows + 7u) / 2u)) % (Rows + 7u);
                        const int distance = streamHead - y;
                        const uint8_t level = distance == 0 ? 255u :
                            distance > 0 && distance <= 6 ? 180u / (distance + 1u) : 0u;
                        if (level > brightness) brightness = level;
                    }
                    color = dim(dim(green, brightness), rainLevel);
                }
                pixels[y * Columns + x] = color;
            }
        }
        if (elapsed >= revealEnd) {
            minuteColor = dim(green, (elapsed - revealEnd) * 255u / (rainEnd - revealEnd));
        }
    }
    for (uint8_t i = 0; i < MinuteLeds; ++i) {
        pixels[Rows * Columns + i] = (minuteFrame->minutes & (1u << i)) ? minuteColor : 0;
    }
}

uint32_t Animator::duration() const {
    if (playingEffect_ == Effect::Noise) {
        return 1000u + (noiseLetterCount_ ? (noiseLetterCount_ - 1u) * 200u : 0u) + 200u;
    }
    return playingEffect_ == Effect::Matrix ? 3200u : 1200u;
}

bool Animator::render(uint32_t now, uint32_t* pixels) {
    if (!initialized_ || pixels == nullptr) return false;
    const uint32_t elapsed = now - startedAt_; // Safe across millis() rollover.
    if (active_ && elapsed >= duration()) {
        active_ = false;
        dirty_ = true;
    }
    if (!dirty_) {
        if (!active_) return false;
        if (playingEffect_ == Effect::Noise) {
            if (elapsed / 50u == uint32_t(renderedAt_ - startedAt_) / 50u) return false;
        } else if (uint32_t(now - renderedAt_) < 33u) return false;
    }
    renderedAt_ = now;
    dirty_ = false;

    if (active_ && playingEffect_ == Effect::Noise) {
        // Keep each noise pattern for 50 ms, even if brightness/color forces a redraw.
        const uint32_t revealEnd = duration() - 200u;
        const uint32_t patternTime = elapsed < revealEnd ? elapsed : revealEnd;
        uint32_t state = (noiseState_ ^ ((patternTime / 50u + 1u) * 0x9e3779b9u)) | 1u;
        uint32_t noiseColor = dim(color_, 153); // 60% (40% reduction); settled letters use full color.
        const uint32_t revealed = elapsed < 1000u ? 0u : 1u + (elapsed - 1000u) / 200u;
        const uint32_t settledCount = revealed < noiseLetterCount_ ? revealed : noiseLetterCount_;
        const uint8_t density = noiseLetterCount_ ? 50u - 40u * settledCount / noiseLetterCount_ : 50u;
        if (elapsed >= revealEnd) {
            // Freeze the final sparse pattern and fade it out over 200 ms.
            noiseColor = dim(noiseColor, (200u - (elapsed - revealEnd)) * 255u / 200u);
        }
        for (uint16_t i = 0; i < PixelCount; ++i) {
            nextRandom(state);
            const bool settled = i < Rows * Columns && noiseRanks_[i] != 255 &&
                noiseRanks_[i] < revealed;
            pixels[i] = settled ? color_ : (state % 100u < density) ? noiseColor : 0;
        }
        return true;
    }

    if (active_ && playingEffect_ == Effect::Matrix) {
        renderMatrix(elapsed, pixels);
        return true;
    }

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

        switch (playingEffect_) {
        case Effect::Fadeout:
            draw(frame, minuteColor, pixels);
            break;
        case Effect::LeftRight: {
            // Alternate occupied rows only; empty rows do not change direction.
            const int distance = phase * Columns / half;
            draw(frame, color_, pixels, entering ? distance - Columns : distance);
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
