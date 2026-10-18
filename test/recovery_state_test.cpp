#include "recovery_state.h"
#include <assert.h>
#include <stdio.h>
#include <initializer_list>
using namespace RecoveryState;
int main() {
    for (auto phase : {Phase::Healthy, Phase::Starting, Phase::Latched}) {
        auto record = make(phase);
        assert(valid(record));
        assert(mustRecover(record, true, true)); // Any exception or WDT.
        assert(mustRecover(record, true, false) == (phase != Phase::Healthy));
        record.check ^= 1;
        assert(!valid(record));
    }
    Record empty{};
    assert(!valid(empty));
    assert(!mustRecover(empty, false, false)); // First power-on gets OTA window.
    assert(mustRecover(empty, false, true)); // Crash still caught without RTC.
    auto starting = make(Phase::Starting);
    assert(mustRecover(starting, true, false)); // Even early software restarts.
    auto healthy = make(Phase::Healthy);
    assert(!mustRecover(healthy, true, false)); // Explicit resume/healthy reboot.
    puts("Recovery state tests passed");
}
