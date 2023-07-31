#include "accept.h"
#include "board.h"

using namespace remix;

auto read() -> uint8_t {
  while (!board::usart.canReceive())
    ;
  return board::usart.receive();
}

auto write(uint8_t c) -> void {
  while (!board::usart.canSend())
    ;
  board::usart.send(c);
}

auto a = accept::Accept<10, write>();

auto main() -> int {
  board::init();
  board::led.init();
  board::led.set();
  board::tx.init();
  board::rx.init();
  board::usart.init();

  board::usart.write(std::span{"\n---- Accept ------------------------\n"});
  bool editing = true;
  while (editing) {
    switch (a.handle(read())) {
    case accept::Canceled:
      board::usart.write(std::span{"\nCanceled.\n"});
      editing = false;
      break;
    case accept::Accepted:
      board::usart.write(std::span{"\nDone: "});
      board::usart.write(a.accepted());
      write('\n');
      editing = false;
      break;
    default:
      break;
    }
  }

  return 0;
}
