#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "key.h"

namespace accept {

enum State {
    Accepted,
    Editing,
    Up,
    Down,
    Tab,
    Canceled,
};

template<typename Output, size_t N = 80>
struct Accept {
    Output& output;
    size_t last { 0 };
    std::array<uint8_t, N> line {};
    key::Reader reader {};

    auto handle(uint8_t key) -> State
    {
        switch (reader.handle(key)) {
        case key::Plain:
            if (last < N) {
                line[last] = reader.value;
                ++last;
                output.send(reader.value);
            }
            break;
        case key::C0:
            switch (reader.value) {
            case 3:
                return Canceled;
            case 9:
                return Tab;
            case 10:
                return Accepted;
            case 8:
            case 127:
                if (last > 0) {
                    --last;
                    output.send(8);
                    output.send(32);
                    output.send(8);
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        return Editing;
    }

    auto accepted() -> std::span<uint8_t> { return std::span<uint8_t>(line.begin(), last); }
    auto reset() -> void { last = 0; }
};

}; // namespace accept
