#include "clock_animation.h"

#include <assert.h>
#include <initializer_list>
#include <stdio.h>

using namespace ClockAnimation;

static constexpr uint32_t Color = 0xcc8040;
static constexpr uint32_t NoiseColor = 0x7a4c26;
static Frame oldFrame() {
    Frame f;
    f.rows[2] = 1u << 2;
    f.minutes = 8;
    return f;
}
static Frame newFrame() {
    Frame f;
    f.rows[7] = 1u << 8;
    f.minutes = 12;
    return f;
}

static void expectFrame(const uint32_t* pixels, const Frame& frame) {
    for (int y = 0; y < Rows; ++y) {
        for (int x = 0; x < Columns; ++x) {
            assert(pixels[y * Columns + x] == (frame.pixel(x, y) ? Color : 0));
        }
    }
    for (int i = 0; i < MinuteLeds; ++i) {
        assert(pixels[Rows * Columns + i] == ((frame.minutes & (1u << i)) ? Color : 0));
    }
}

static Animator prepare(Effect effect, uint32_t* pixels, uint32_t now = 0) {
    Animator a;
    a.setColor(Color);
    a.setEffect(effect);
    a.setFrame(oldFrame(), now);
    assert(!a.active()); // Boot must never animate from an empty display.
    assert(a.render(now, pixels));
    expectFrame(pixels, oldFrame());
    return a;
}

int main() {
    uint32_t pixels[PixelCount];
    for (uint8_t i = 0; i < OptionCount; ++i) {
        Effect effect = Effect::None;
        assert(parseEffect(Options[i].id, effect));
        assert(effect == Options[i].effect);
    }
    Effect invalid = Effect::Matrix;
    assert(!parseEffect("unknown", invalid) && invalid == Effect::Matrix);
    assert(!parseEffect(nullptr, invalid));
    assert(!oldFrame().pixel(-1, 0) && !oldFrame().pixel(Columns, Rows));

    auto plain = prepare(Effect::None, pixels);
    plain.setFrame(newFrame(), 100);
    assert(!plain.active() && plain.render(100, pixels));
    expectFrame(pixels, newFrame());
    plain.setFrame(newFrame(), 1000);
    assert(!plain.render(1000, pixels));

    auto fade = prepare(Effect::Fadeout, pixels);
    fade.setFrame(newFrame(), 100);
    assert(fade.render(100, pixels));
    expectFrame(pixels, oldFrame());
    assert(!fade.render(120, pixels)); // Rate limit without waiting.
    assert(fade.render(400, pixels));
    assert(pixels[2 * Columns + 2] == 0x653f1f);
    assert(fade.render(700, pixels));
    for (auto pixel : pixels) assert(pixel == 0);
    assert(fade.render(1000, pixels));
    assert(pixels[7 * Columns + 8] == 0x653f1f);
    // The one-second RTC poll must not restart an in-progress transition.
    fade.setFrame(newFrame(), 1100);
    assert(fade.render(1300, pixels) && !fade.active());
    expectFrame(pixels, newFrame());

    Frame slideOld, slideNew;
    slideOld.minutes = 8;
    slideNew.minutes = 12;
    for (uint8_t y = 0; y < Rows; ++y) {
        slideOld.rows[y] = 1u << ((y & 1u) ? 8 : 2);
        slideNew.rows[y] = 1u << ((y & 1u) ? 2 : 8);
    }
    Animator slide;
    slide.setColor(Color);
    slide.setEffect(Effect::LeftRight);
    slide.setFrame(slideOld, 0);
    assert(slide.render(0, pixels) && !slide.active());
    expectFrame(pixels, slideOld);
    slide.setFrame(slideNew, 100);
    assert(slide.render(400, pixels));
    for (uint8_t y = 0; y < Rows; ++y) {
        for (uint8_t x = 0; x < Columns; ++x) {
            const uint8_t expectedX = (y & 1u) ? 3 : 7;
            assert(pixels[y * Columns + x] == (x == expectedX ? Color : 0));
        }
    }
    assert(slide.render(650, pixels));
    for (uint16_t i = 0; i < Rows * Columns; ++i) assert(pixels[i] == 0); // clip, never wrap
    assert(slide.render(700, pixels));
    for (auto pixel : pixels) assert(pixel == 0);
    assert(slide.render(1000, pixels));
    for (uint8_t y = 0; y < Rows; ++y) {
        for (uint8_t x = 0; x < Columns; ++x) {
            const uint8_t expectedX = (y & 1u) ? 8 : 2;
            assert(pixels[y * Columns + x] == (x == expectedX ? Color : 0));
        }
    }
    assert(slide.render(1300, pixels) && !slide.active());
    expectFrame(pixels, slideNew);

    // Only occupied rows count: physical rows 1, 2, 4 exit right, left, right.
    Frame sparseOld, sparseNew;
    sparseOld.rows[0] = 1u << 2;
    sparseOld.rows[1] = 1u << 8;
    sparseOld.rows[3] = 1u << 2;
    // Incoming text has different occupied rows and starts below an empty row.
    sparseNew.rows[1] = 1u << 8;
    sparseNew.rows[4] = 1u << 2;
    sparseNew.rows[8] = 1u << 8;
    Animator sparseSlide;
    sparseSlide.setColor(Color);
    sparseSlide.setEffect(Effect::LeftRight);
    sparseSlide.setFrame(sparseOld, 0);
    assert(sparseSlide.render(0, pixels) && !sparseSlide.active());
    expectFrame(pixels, sparseOld);
    sparseSlide.setFrame(sparseNew, 100);
    assert(sparseSlide.render(400, pixels));
    Frame sparseExpected;
    sparseExpected.rows[0] = 1u << 7;
    sparseExpected.rows[1] = 1u << 3;
    sparseExpected.rows[3] = 1u << 7;
    expectFrame(pixels, sparseExpected);
    assert(sparseSlide.render(1000, pixels));
    sparseExpected = Frame{};
    sparseExpected.rows[1] = 1u << 2;
    sparseExpected.rows[4] = 1u << 8;
    sparseExpected.rows[8] = 1u << 2;
    expectFrame(pixels, sparseExpected);
    assert(sparseSlide.render(1300, pixels) && !sparseSlide.active());
    expectFrame(pixels, sparseNew);

    auto matrix = prepare(Effect::Matrix, pixels);
    matrix.setFrame(newFrame(), 100);
    assert(matrix.render(500, pixels));
    assert(pixels[2 * Columns + 2] != 0); // staggered column just starts
    assert(matrix.render(700, pixels));
    assert(pixels[4 * Columns + 2] == 0x90ffb0); // falls downward
    assert(pixels[3 * Columns + 2] == 0x006419); // trailing green above
    assert(matrix.render(1700, pixels) && matrix.active()); // still active after old duration
    assert(matrix.render(3267, pixels) && matrix.active());
    assert(matrix.render(3300, pixels) && !matrix.active());
    expectFrame(pixels, newFrame()); // selected color returns

    auto noise = prepare(Effect::Noise, pixels);
    noise.setFrame(newFrame(), 100);
    uint32_t previousNoise[PixelCount] = {};
    for (uint32_t elapsed = 0; elapsed < 1000; elapsed += 50) {
        // Repeated RTC polls must not restart the transition.
        noise.setFrame(newFrame(), 100 + elapsed);
        assert(noise.render(100 + elapsed, pixels) && noise.active());
        unsigned lit = 0;
        bool changed = false;
        for (uint16_t i = 0; i < PixelCount; ++i) {
            assert(pixels[i] == 0 || pixels[i] == NoiseColor);
            lit += pixels[i] != 0;
            changed = changed || pixels[i] != previousNoise[i];
            previousNoise[i] = pixels[i];
        }
        assert(lit > 0 && lit < PixelCount && changed);
        assert(!noise.render(149 + elapsed, pixels));
        noise.requestRedraw();
        assert(noise.render(149 + elapsed, pixels));
        for (uint16_t i = 0; i < PixelCount; ++i) assert(pixels[i] == previousNoise[i]);
    }
    assert(noise.render(1300, pixels) && !noise.active());
    expectFrame(pixels, newFrame());

    // Letters settle at 200 ms intervals; noise gets sparser, then fades for 200 ms.
    bool randomFirstObserved = false;
    for (uint32_t start : {100u, 1234u, 5678u, UINT32_MAX - 600u}) {
        auto reveal = prepare(Effect::Noise, pixels, start - 1);
        Frame text;
        text.rows[1] = (1u << Columns) - 1u;
        text.rows[8] = 1u << 3;
        reveal.setFrame(text, start);
        bool knownSettled[PixelCount] = {};
        unsigned initialNoise = 0, lateNoise = 0;
        for (uint32_t elapsed = 0; elapsed < 3200; elapsed += 50) {
            assert(reveal.render(start + elapsed, pixels) && reveal.active());
            unsigned settledCount = 0;
            for (uint16_t i = 0; i < PixelCount; ++i) {
                assert(pixels[i] == 0 || pixels[i] == NoiseColor || pixels[i] == Color);
                const bool targetLetter = i < Rows * Columns && text.pixel(i % Columns, i / Columns);
                if (!targetLetter) {
                    assert(pixels[i] != Color);
                    if (elapsed < 200) initialNoise += pixels[i] != 0;
                    if (elapsed >= 3000) lateNoise += pixels[i] != 0;
                    continue;
                }
                if (knownSettled[i]) assert(pixels[i] == Color);
                if (pixels[i] == Color) {
                    knownSettled[i] = true;
                    if (elapsed == 1000 && i != Columns) randomFirstObserved = true;
                }
                settledCount += knownSettled[i];
            }
            const unsigned revealed = elapsed < 1000 ? 0 : 1 + (elapsed - 1000) / 200;
            assert(settledCount == revealed);
        }
        assert(initialNoise > lateNoise * 2); // Equal sample counts, density falls from 50% to ~10%.
        uint32_t previousPixels[PixelCount];
        unsigned fadingPixels = 0;
        for (uint32_t elapsed = 3200; elapsed < 3400; elapsed += 50) {
            assert(reveal.render(start + elapsed, pixels) && reveal.active());
            for (uint16_t i = 0; i < PixelCount; ++i) {
                const bool targetLetter = i < Rows * Columns && text.pixel(i % Columns, i / Columns);
                if (targetLetter) assert(pixels[i] == Color);
                else if (elapsed == 3200) fadingPixels += pixels[i] != 0;
                else {
                    if (previousPixels[i]) assert(pixels[i] < previousPixels[i]);
                    else assert(pixels[i] == 0); // The fading pattern never jumps or adds pixels.
                }
                previousPixels[i] = pixels[i];
            }
        }
        assert(fadingPixels > 0);
        assert(reveal.render(start + 3400, pixels) && !reveal.active());
        expectFrame(pixels, text);
    }
    assert(randomFirstObserved);

    // Random selects a concrete animated effect once and keeps it throughout the transition.
    auto random = prepare(Effect::Random, pixels);
    const Effect candidates[] = {Effect::Fadeout, Effect::Matrix, Effect::LeftRight, Effect::Noise};
    bool seen[4] = {};
    const uint32_t times[] = {0, 400, 800, 1100, 1600, 3300};
    for (uint32_t trial = 0; trial < 64; ++trial) {
        const uint32_t start = 100 + trial * 5000;
        uint32_t snapshots[6][PixelCount];
        bool active[6];
        random.preview(Effect::Random, start);
        for (unsigned t = 0; t < 6; ++t) {
            random.render(start + times[t], pixels);
            for (unsigned i = 0; i < PixelCount; ++i) snapshots[t][i] = pixels[i];
            active[t] = random.active();
        }
        unsigned matches = 0;
        for (unsigned c = 0; c < 4; ++c) {
            auto concrete = prepare(Effect::None, pixels);
            concrete.preview(candidates[c], start);
            bool equal = true;
            for (unsigned t = 0; t < 6; ++t) {
                concrete.render(start + times[t], pixels);
                equal = equal && active[t] == concrete.active();
                for (unsigned i = 0; i < PixelCount; ++i) equal = equal && pixels[i] == snapshots[t][i];
            }
            if (equal) { ++matches; seen[c] = true; }
        }
        assert(matches == 1); // Neither None nor a re-selection mid-animation is allowed.
    }
    for (bool selected : seen) assert(selected);

    // Every effect must render only in bounds, finish, and handle millis wrap.
    for (uint8_t i = 0; i < OptionCount; ++i) {
        const uint32_t start = UINT32_MAX - 500;
        auto a = prepare(Options[i].effect, pixels, start - 1);
        a.setFrame(newFrame(), start);
        for (uint32_t elapsed = 0; elapsed <= 6000; ++elapsed) {
            a.render(start + elapsed, pixels);
        }
        assert(!a.active());
        expectFrame(pixels, newFrame());
        assert(!a.render(start + 6001, pixels));
        a.requestRedraw(); // brightness change regenerates original RGB values
        assert(a.render(start + 6002, pixels));
        expectFrame(pixels, newFrame());
    }

    auto interrupted = prepare(Effect::Fadeout, pixels);
    interrupted.setFrame(newFrame(), 100);
    interrupted.setEffect(Effect::None);
    assert(interrupted.render(200, pixels) && !interrupted.active());
    expectFrame(pixels, newFrame());

    auto minuteOnly = prepare(Effect::Fadeout, pixels);
    Frame minute = oldFrame();
    minute.minutes = 12;
    minuteOnly.setFrame(minute, 100);
    assert(!minuteOnly.active() && minuteOnly.render(100, pixels));
    expectFrame(pixels, minute);
    minuteOnly.setFrame(newFrame(), 200); // time jump replaces destination
    assert(minuteOnly.active());
    assert(minuteOnly.render(1400, pixels) && !minuteOnly.active());
    expectFrame(pixels, newFrame());
    minuteOnly.setColor(0x123456);
    assert(minuteOnly.render(1401, pixels));
    assert(pixels[7 * Columns + 8] == 0x123456);
    assert(pixels[Rows * Columns + 3] == 0x123456);

    // All effects leave the words still while the four minute dots advance.
    for (uint8_t i = 0; i < OptionCount; ++i) {
        auto a = prepare(Options[i].effect, pixels);
        Frame current = oldFrame();
        uint32_t now = 60000;
        for (uint8_t dots : {12, 14, 15, 0}) {
            current.minutes = dots;
            a.setFrame(current, now);
            assert(!a.active() && a.render(now, pixels));
            expectFrame(pixels, current);
            a.setFrame(current, now + 1000);
            assert(!a.render(now + 1000, pixels));
            now += 60000;
        }
        // A text change animates even when the minute dots remain identical.
        Frame changed = newFrame();
        changed.minutes = current.minutes;
        a.setFrame(changed, now);
        assert(a.active() == (Options[i].effect != Effect::None));
        assert(a.render(now + 5000, pixels) && !a.active());
        expectFrame(pixels, changed);
    }

    // A dot update during an existing word transition preserves its deadline.
    auto ongoing = prepare(Effect::Matrix, pixels);
    ongoing.setFrame(newFrame(), 100);
    assert(ongoing.render(500, pixels) && ongoing.active());
    Frame newDots = newFrame();
    newDots.minutes = 1;
    ongoing.setFrame(newDots, 1000);
    assert(ongoing.active());
    assert(ongoing.render(3300, pixels) && !ongoing.active());
    expectFrame(pixels, newDots);

    // Preview works without a text change, even with the saved effect disabled.
    for (uint8_t i = 0; i < OptionCount; ++i) {
        auto a = prepare(Effect::None, pixels);
        a.preview(Options[i].effect, 100);
        assert(a.active() == (Options[i].effect != Effect::None));
        assert(a.render(100, pixels));
        a.setFrame(oldFrame(), 1000); // RTC poll preserves the preview.
        assert(a.render(5100, pixels) == (Options[i].effect != Effect::None));
        assert(!a.active());
        expectFrame(pixels, oldFrame());
        a.setFrame(newFrame(), 6000); // Preview never changes the saved effect.
        assert(a.render(6000, pixels) && !a.active());
        expectFrame(pixels, newFrame());
    }
    auto preview = prepare(Effect::Matrix, pixels);
    preview.preview(Effect::Noise, 100);
    preview.preview(Effect::Fadeout, 200); // A new selection replaces a running test.
    assert(preview.render(1400, pixels) && !preview.active());
    expectFrame(pixels, oldFrame());
    preview.setFrame(newFrame(), 1500);
    assert(preview.render(4700, pixels) && !preview.active()); // Saved Matrix duration.
    expectFrame(pixels, newFrame());
    preview.preview(Effect::Noise, 5000);
    preview.preview(Effect::None, 5001);
    assert(preview.render(5001, pixels) && !preview.active());
    expectFrame(pixels, newFrame());

    puts("Animation tests passed");
}
