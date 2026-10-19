// Incremental RFC 6455 client frame header reader. No heap or hardware dependency.
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace WordclockWebSocket {
class FrameHeader {
public:
    bool consume(uint8_t*& data, size_t& available) {
        while (available && used_ < needed_) {
            size_t take = needed_ - used_;
            if (take > available) take = available;
            memcpy(bytes_ + used_, data, take);
            used_ += take;
            data += take;
            available -= take;
            if (used_ == 2 && needed_ == 2) {
                const uint8_t length = bytes_[1] & 0x7f;
                needed_ = 2 + (length == 126 ? 2 : length == 127 ? 8 : 0) +
                          ((bytes_[1] & 0x80) ? 4 : 0);
            }
        }
        return used_ == needed_;
    }

    void reset() { used_ = 0; needed_ = 2; }
    bool final() const { return (bytes_[0] & 0x80) != 0; }
    uint8_t opcode() const { return bytes_[0] & 0x0f; }
    bool masked() const { return (bytes_[1] & 0x80) != 0; }
    uint64_t length() const {
        uint64_t value = bytes_[1] & 0x7f;
        if (value < 126) return value;
        const uint8_t count = value == 126 ? 2 : 8;
        value = 0;
        for (uint8_t i = 0; i < count; ++i) value = (value << 8) | bytes_[2 + i];
        return value;
    }
    const uint8_t* mask() const { return bytes_ + needed_ - 4; }
    bool validClientFrame() const {
        if (!masked() || (bytes_[0] & 0x70) || (length() >> 63)) return false;
        const uint8_t op = opcode();
        if (op != 0 && op != 1 && op != 2 && op != 8 && op != 9 && op != 10) return false;
        if (op >= 8 && (!final() || length() > 125 || (op == 8 && length() == 1))) return false;
        return true;
    }

private:
    uint8_t bytes_[14]{};
    uint8_t used_ = 0;
    uint8_t needed_ = 2;
};
} // namespace WordclockWebSocket
