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

struct Result {
  State state;
  std::span<const uint8_t> output{};
};

template <size_t N> class Accept {
public:
  auto handle(uint8_t key) -> Result {
    switch (reader.handle(key)) {
    case key::Plain:
      if (last < N) {
        line[last] = reader.value;
        ++last;
        return {.state = Editing, .output = std::span(&line[last - 1], &line[last])};
      }
      break;
    case key::C0:
      switch (reader.value) {
      case 3:
        return {.state = Canceled};
      case 9:
        return {.state = Tab};
      case 10:
        return {.state = Accepted};
      case 8:
      case 127:
        if (last > 0) {
          --last;
          return {.state = Editing, .output = std::span(backspace)};
        }
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
    return {.state = Editing};
  }

  auto accepted() -> std::span<uint8_t> { return std::span<uint8_t>(line.begin(), last); }

  auto reset() -> void { last = 0; }

protected:
  static constexpr uint8_t backspace[] = {8, 32, 8};
  size_t last{0};
  std::array<uint8_t, N> line;
  key::Reader reader;
};

}; // namespace accept
