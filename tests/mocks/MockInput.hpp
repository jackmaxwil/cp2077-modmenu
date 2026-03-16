#pragma once

#include "MockRED4ext.hpp"
#include "MockRed4.hpp"
#include <cstdint>
#include <functional>

namespace Mock {

using KeyCode = uint32_t;
using InputAction = uint32_t;

struct InputEvent {
    KeyCode keyCode = 0;
    bool isDown = false;
    bool isUp = false;
    bool isRepeat = false;
    uint64_t timestamp = 0;
};

struct InputConsumer {
    void BindAction(KeyCode keyCode, const std::function<void(const InputEvent&)>& callback);
    void UnbindAction(KeyCode keyCode);
    
    std::unordered_map<KeyCode, std::function<void(const InputEvent&)>> bindings;
};

struct InputManager {
    static InputManager* Get();
    
    static std::function<void(const InputEvent&)> s_inputCallback;
    
    void ProcessEvent(const InputEvent& event);
    void RegisterCallback(const std::function<void(const InputEvent&)>& callback);
    void ClearCallbacks();
};

} // namespace Mock