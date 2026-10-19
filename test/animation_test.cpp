#include "clock_animation.h"

#include <assert.h>
#include <initializer_list>
#include <stdio.h>

using namespace ClockAnimation;

static constexpr uint32_t Color = 0xcc8040;
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

    auto slide = prepare(Effect::LeftRight, pixels);
    slide.setFrame(newFrame(), 100);
    assert(slide.render(400, pixels));
    assert(pixels[2 * Columns + 7] == Color); // old x=2 moves right by 5
    assert(pixels[2 * Columns + 2] == 0);
    assert(slide.render(1000, pixels));
    assert(pixels[7 * Columns + 2] == Color); // new x=8 enters at x=2

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

    // Every effect must render only in bounds, finish, and handle millis wrap.
    for (uint8_t i = 0; i < OptionCount; ++i) {
        const uint32_t start = UINT32_MAX - 500;
        auto a = prepare(Options[i].effect, pixels, start - 1);
        a.setFrame(newFrame(), start);
        for (uint32_t elapsed = 0; elapsed <= 4000; ++elapsed) {
            a.render(start + elapsed, pixels);
        }
        assert(!a.active());
        expectFrame(pixels, newFrame());
        assert(!a.render(start + 5000, pixels));
        a.requestRedraw(); // brightness change regenerates original RGB values
        assert(a.render(start + 5001, pixels));
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
        assert(a.render(now + 4000, pixels) && !a.active());
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

    puts("Animation tests passed");
}
