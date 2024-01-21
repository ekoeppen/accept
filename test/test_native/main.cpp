#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include <ranges>
#include <span>

#include "accept.h"
#include "key.h"
#include "states.h"

using namespace accept;

template <size_t N> auto equals(std::span<uint8_t> a, const uint8_t (&b)[N]) -> bool {
  if (a.size() != N)
    return false;
  for (size_t i = 0; i < N; i++) {
    if (a[i] != b[i])
      return false;
  }
  return true;
}

TEST_CASE("FSM States") {
  fsm::Fsm<uint8_t, states::State, states::Action, states::transitions> fsm{
      .state = states::Input,
  };
  SUBCASE("Simple inputs") {
    for (uint8_t k : std::ranges::views::iota(32, 255) |
                         std::views::filter([](uint8_t k) { return k != 27; })) {
      auto action = fsm.input(k);
      CHECK(action.hasValue);
      CHECK(action.value == states::Plain);
      CHECK(fsm.state == states::Input);
    }
  }

  SUBCASE("C1 control codes") {
    for (uint8_t k : std::views::iota(0, 255) |
                         std::views::filter([](uint8_t k) { return k != 'O' && k != '['; })) {
      auto action = fsm.input(27);
      CHECK(!action.hasValue);
      CHECK(fsm.state == states::Esc);

      action = fsm.input(k);
      CHECK(action.hasValue);
      CHECK(action.value == states::C1);
      CHECK(fsm.state == states::Input);
    }
  }

  SUBCASE("SS3 control codes") {
    for (uint8_t k : std::views::iota(0, 255)) {
      auto action = fsm.input(27);
      CHECK(!action.hasValue);
      CHECK(fsm.state == states::Esc);

      action = fsm.input('O');
      CHECK(!action.hasValue);
      CHECK(fsm.state == states::Vt_App_State);

      action = fsm.input(k);
      CHECK(action.hasValue);
      CHECK(action.value == states::Vt_App);
      CHECK(fsm.state == states::Input);
    }
  }

  SUBCASE("CSI control codes") {
    fsm::Optional<states::Action> action;

    fsm.input(27);
    action = fsm.input('[');
    CHECK(action.hasValue);
    CHECK(action.value == states::Start_Param);
    CHECK(fsm.state == states::Esc_Csi_Param);

    action = fsm.input('1');
    CHECK(action.hasValue);
    CHECK(action.value == states::Update_Param);
    CHECK(fsm.state == states::Esc_Csi_Param);

    action = fsm.input(';');
    CHECK(action.hasValue);
    CHECK(action.value == states::Param_Done);
    CHECK(fsm.state == states::Esc_Csi_Param);

    action = fsm.input('2');
    CHECK(action.hasValue);
    CHECK(action.value == states::Update_Param);
    CHECK(fsm.state == states::Esc_Csi_Param);
  }
}

TEST_CASE("Key handling") {
  auto key = key::Reader();
  auto result = key.handle('A');
  CHECK(result == key::Plain);
  CHECK(key.value == 'A');
  result = key.handle(8);
  CHECK(result == key::C0);
  CHECK(key.value == 8);
  key.handle(27);
  key.handle('O');
  result = key.handle('A');
  CHECK(result == key::Vt_App);
  CHECK(key.value == 'A');
  key.handle(27);
  key.handle('[');
  result = key.handle('A');
  CHECK(result == key::Csi);
  CHECK(key.csi.numParams == 1);
  CHECK(key.csi.params[0] == 0);
  CHECK(key.csi.finalValue == 'A');
  key.handle(27);
  key.handle('[');
  key.handle('1');
  key.handle(';');
  key.handle('2');
  result = key.handle('m');
  CHECK(result == key::Csi);
  CHECK(key.csi.numParams == 2);
  CHECK(key.csi.params[0] == 1);
  CHECK(key.csi.params[1] == 2);
  CHECK(key.csi.finalValue == 'm');
}

TEST_CASE("Accept") {
  struct Writer {
    auto send(uint8_t c) const -> void { (void)c; };
  };
  Writer writer;

  auto accept = accept::Accept<Writer, 4>{.output = writer};
  SUBCASE("Simple keys") {
    auto result = accept.handle('A');
    CHECK(result == accept::Editing);
    CHECK(equals(accept.accepted(), {'A'}));
    result = accept.handle('B');
    CHECK(equals(accept.accepted(), {'A', 'B'}));
  }
  SUBCASE("Limit") {
    accept.handle('1');
    accept.handle('2');
    accept.handle('3');
    accept.handle('4');
    accept.handle('5');
    CHECK(equals(accept.accepted(), {'1', '2', '3', '4'}));
  }
  SUBCASE("Backspace") {
    accept.handle('1');
    accept.handle('2');
    accept.handle(8);
    accept.handle('3');
    accept.handle('4');
    accept.handle(127);
    CHECK(equals(accept.accepted(), {'1', '3'}));
  }
  SUBCASE("Backspace limit") {
    accept.handle(8);
    accept.handle('1');
    accept.handle(8);
    accept.handle(127);
    CHECK(accept.accepted().size() == 0);
  }
}

int main(int argc, char **argv) {
  doctest::Context context;
  context.setOption("success", true);
  context.setOption("no-exitcode", true);
  context.applyCommandLine(argc, argv);
  return context.run();
}
