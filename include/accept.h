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

template <size_t N, typename Writer> struct Accept {
  auto handle(uint8_t key) -> State {
    switch (reader.handle(key)) {
    case key::Plain:
      if (last < N) {
        line[last] = reader.value;
        ++last;
        writer.write(reader.value);
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
          writer.write(8);
          writer.write(32);
          writer.write(8);
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

  const Writer &writer;
  size_t last{0};
  std::array<uint8_t, N> line{};
  key::Reader reader{};
};

}; // namespace accept
