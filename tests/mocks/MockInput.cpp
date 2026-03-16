#include "MockInput.hpp"

namespace Mock {

void InputConsumer::BindAction(KeyCode keyCode, const std::function<void(const InputEvent&)>& callback) {
    bindings[keyCode] = callback;
}

void InputConsumer::UnbindAction(KeyCode keyCode) {
    bindings.erase(keyCode);
}

InputManager* InputManager::Get() {
    static InputManager manager;
    return &manager;
}

void InputManager::ProcessEvent(const InputEvent& event) {
    if (s_inputCallback) {
        s_inputCallback(event);
    }
}

void InputManager::RegisterCallback(const std::function<void(const InputEvent&)>& callback) {
    s_inputCallback = callback;
}

void InputManager::ClearCallbacks() {
    s_inputCallback = nullptr;
}

std::function<void(const InputEvent&)> InputManager::s_inputCallback = nullptr;

} // namespace Mock