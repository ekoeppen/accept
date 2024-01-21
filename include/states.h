#pragma once

#include <array>
#include <cstdint>

#include "fsm.h"

namespace accept::states {

enum State {
    Input,
    Esc,
    Esc_Csi_Param,
    Vt_App_State,
};

enum Action {
    Plain,
    C1,
    Start_Param,
    Update_Param,
    Param_Done,
    Vt_App,
};

constexpr std::array<fsm::Transition<uint8_t, State, Action>, 8> transitions { {
    { .state = Input, .event = 27, .newState = Esc },
    { .state = Input, .action = Plain },
    { .state = Esc, .event = '[', .action = Start_Param, .newState = Esc_Csi_Param },
    { .state = Esc, .event = 'O', .newState = Vt_App_State },
    { .state = Esc, .action = C1, .newState = Input },
    { .state = Esc_Csi_Param, .event = ';', .action = Param_Done, .newState = Esc_Csi_Param },
    { .state = Esc_Csi_Param, .action = Update_Param },
    { .state = Vt_App_State, .action = Vt_App, .newState = Input },
} };

}; // namespace accept::states
