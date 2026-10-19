#include "WebSocketFrameHeader.h"
#include <assert.h>
#include <stdio.h>
#include <vector>

using WordclockWebSocket::FrameHeader;

static std::vector<uint8_t> header(uint64_t length, uint8_t opcode = 2) {
    std::vector<uint8_t> result{uint8_t(0x80 | opcode)};
    if (length < 126) result.push_back(uint8_t(0x80 | length));
    else {
        const int count = length <= 65535 ? 2 : 8;
        result.push_back(count == 2 ? 0xfe : 0xff);
        for (int i = count - 1; i >= 0; --i) result.push_back(uint8_t(length >> (i * 8)));
    }
    for (uint8_t value : {1, 2, 3, 4}) result.push_back(value);
    return result;
}

int main() {
    for (uint64_t length : {0ull, 2ull, 125ull, 126ull, 65535ull, 65536ull, 0x100000000ull}) {
        auto bytes = header(length);
        // Every possible single split, including a lone first byte and partial mask/length.
        for (size_t split = 0; split <= bytes.size(); ++split) {
            FrameHeader reader;
            uint8_t* data = bytes.data();
            size_t available = split;
            const bool done = reader.consume(data, available);
            assert(done == (split == bytes.size()));
            assert(available == 0 && data == bytes.data() + split);
            available = bytes.size() - split;
            assert(reader.consume(data, available));
            assert(available == 0 && data == bytes.data() + bytes.size());
            assert(reader.length() == length && reader.validClientFrame());
            assert(reader.opcode() == 2 && reader.final());
            for (uint8_t i = 0; i < 4; ++i) assert(reader.mask()[i] == i + 1);
        }
        // Byte-by-byte headers followed by payload in the same TCP buffer.
        FrameHeader reader;
        for (size_t i = 0; i < bytes.size(); ++i) {
            uint8_t* data = &bytes[i];
            size_t available = 1;
            assert(reader.consume(data, available) == (i + 1 == bytes.size()));
            assert(available == 0);
        }
        reader.reset();
        bytes.push_back(0xaa);
        uint8_t* data = bytes.data();
        size_t available = bytes.size();
        assert(reader.consume(data, available));
        assert(available == 1 && *data == 0xaa);
    }
    for (uint8_t op : {8, 9, 10}) {
        for (uint64_t length : {0ull, 1ull, 2ull, 125ull, 126ull}) {
            auto bytes = header(length, op);
            FrameHeader reader;
            uint8_t* data = bytes.data(); size_t available = bytes.size();
            assert(reader.consume(data, available));
            assert(reader.validClientFrame() == (length <= 125 && !(op == 8 && length == 1)));
            bytes[0] &= 0x7f; reader.reset(); data = bytes.data(); available = bytes.size();
            assert(reader.consume(data, available) && !reader.validClientFrame());
        }
    }
    for (int invalid = 0; invalid < 4; ++invalid) {
        auto bytes = header(invalid == 3 ? 0x8000000000000000ull : 2);
        if (invalid == 0) bytes[0] |= 0x40; // No compression extension was negotiated.
        if (invalid == 1) bytes[0] = 0x8f; // Reserved opcode.
        if (invalid == 2) bytes[1] &= 0x7f; // Client frames must be masked.
        FrameHeader reader;
        uint8_t* data = bytes.data(); size_t available = bytes.size();
        assert(reader.consume(data, available) && !reader.validClientFrame());
    }
    puts("WebSocket header tests passed");
}
