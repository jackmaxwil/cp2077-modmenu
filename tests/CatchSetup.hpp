#pragma once

#include <cstdint>
#include <unordered_map>

#ifdef _MSC_VER
#define CATCH_PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define CATCH_PLATFORM_MACOS
#elif defined(__linux__)
#define CATCH_PLATFORM_LINUX
#endif

#include <catch2/catch_all.hpp>

#define CATCH_CONFIG_MAIN

#include "MockRED4ext.hpp"
#include "MockRed4.hpp"
#include "MockInk.hpp"
#include "MockInput.hpp"

namespace TestHelpers {
    
struct TestEnvironment {
    TestEnvironment() {
        Mock::ClearMockRTTI();
        Mock::ClearMockRedClasses();
        Mock::ClearMockInk();
        Mock::RegisterRTTI(0x12345678, "TestHUDController", sizeof(Mock::RedClass));
        Mock::RegisterRTTI(0x87654321, "inkCompoundWidget", sizeof(Mock::inkCompoundWidget));
    }
    
    ~TestEnvironment() {
        Mock::ClearMockRTTI();
        Mock::ClearMockRedClasses();
        Mock::ClearMockInk();
    }
};

inline Mock::InputEvent CreateKeyPress(uint32_t keyCode, bool isDown) {
    Mock::InputEvent event;
    event.keyCode = keyCode;
    event.isDown = isDown;
    event.isUp = !isDown;
    event.timestamp = 12345;
    return event;
}

}

#define SETUP_TEST_ENVIRONMENT() TestHelpers::TestEnvironment env;