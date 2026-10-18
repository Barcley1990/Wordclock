#pragma once
#include <stdint.h>

namespace RecoveryState {
enum class Phase : uint32_t { Healthy = 0, Starting = 1, Latched = 2 };
struct Record {
    uint32_t magic;
    uint32_t phase;
    uint32_t check;
};
constexpr uint32_t Magic = 0x57435231;
inline Record make(Phase phase) {
    return {Magic, uint32_t(phase), Magic ^ uint32_t(phase) ^ 0xa59c3e71u};
}
inline bool valid(const Record& r) {
    return r.magic == Magic && r.phase <= uint32_t(Phase::Latched) &&
           r.check == (r.magic ^ r.phase ^ 0xa59c3e71u);
}
inline bool mustRecover(const Record& r, bool readOk, bool crashReset) {
    // A missing RTC record is normal after power loss or first installation.
    return crashReset || (readOk && valid(r) && r.phase != uint32_t(Phase::Healthy));
}
} // namespace RecoveryState
