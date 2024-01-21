#include <signal.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>

#include "accept.h"
#include "key.h"
#include "states.h"

static struct termios old_input_options;

auto configure_input() -> void
{
    struct termios options;

    tcgetattr(0, &options);

    old_input_options = options;

    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;
    options.c_cflag |= CLOCAL | CREAD | HUPCL;
    options.c_cflag &= ~(IXON | IXOFF);
    options.c_lflag = 0;

    tcflush(0, TCIFLUSH);
    tcsetattr(0, TCSANOW, &options);
}

auto unconfigure_input() -> void { tcsetattr(0, TCSANOW, &old_input_options); }

auto signal_handler(int signum) -> void
{
    switch (signum) {
    case SIGQUIT:
    case SIGINT:
    case SIGTERM:
        unconfigure_input();
        break;
    default:
        break;
    }
}

auto setup_signal_handlers() -> void
{
    signal(SIGQUIT, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

auto out(uint8_t c) -> void { write(1, &c, 1); }
auto in() -> uint8_t
{
    uint8_t c;
    read(0, &c, 1);
    return c;
}

struct Writer {
    auto send(uint8_t c) const -> void { out(c); }
};

Writer writer;

auto main() -> int
{
    configure_input();
    setup_signal_handlers();

    std::cout << "Accept:" << std::endl;
    auto accept = accept::Accept<Writer, 10> { .output = writer };
    bool done = false;
    while (!done) {
        switch (accept.handle(in())) {
        case accept::Accepted:
            std::cout << std::endl << "Accepted: ";
            for (auto c : accept.accepted()) {
                std::cout << c;
            }
            std::cout << std::endl;
            done = true;
            break;
        case accept::Canceled:
            done = true;
            break;
        default:
            break;
        }
    }
    unconfigure_input();
    return 0;
}
