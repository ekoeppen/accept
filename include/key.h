#pragma once

#include <array>

#include "states.h"

namespace accept::key {

enum Result {
    Plain,
    C0,
    C1,
    Vt_App,
    Csi,
    Incomplete,
};

struct Csi {
    std::array<uint16_t, 3> params { 0, 0, 0 };
    uint8_t numParams { 0 };
    uint8_t finalValue { 0 };
};

struct Reader {
    struct Csi csi {};
    uint8_t value;

    fsm::Fsm<uint8_t, states::State, states::Action, states::transitions> fsm {
        .state = states::Input,
    };

    auto handle(uint8_t key) -> Result
    {
        auto action = fsm.input(key);
        if (!action.hasValue) {
            return Incomplete;
        }
        value = key;
        switch (action.value) {
        case states::Plain:
            if (key >= ' ' && key != 127) {
                return Plain;
            }
            return C0;
        case states::C1:
            return C1;
        case states::Vt_App:
            return Vt_App;
        case states::Start_Param:
            csi.numParams = 0;
            csi.finalValue = 0;
            return Incomplete;
        case states::Update_Param:
            if (key < '0' || key > '9') {
                csi.finalValue = key;
                ++csi.numParams;
                fsm.state = states::Input;
                return Csi;
            } else if (csi.numParams < csi.params.size()) {
                csi.params[csi.numParams] *= 10;
                csi.params[csi.numParams] += static_cast<uint16_t>(key - 48);
            }
            return Incomplete;
        case states::Param_Done:
            ++csi.numParams;
            return Incomplete;
        default:
            return Incomplete;
        }
    }
};

}; // namespace accept::key
