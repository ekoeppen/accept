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

template <size_t N, auto write> class Accept {
public:
  Accept() : last(0) {}

  auto handle(uint8_t key) -> State {
    switch (reader.handle(key)) {
    case key::Plain:
      if (last < N) {
        write(reader.value);
        line[last] = reader.value;
        ++last;
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
          write(8);
          write(' ');
          write(8);
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

protected:
  size_t last;
  std::array<uint8_t, N> line;
  key::Reader reader;
};

}; // namespace accept
