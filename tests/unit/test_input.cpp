#include "../CatchSetup.hpp"
#include "../mocks/MockInput.hpp"
#include <functional>

TEST_CASE("Input Event Creation", "[input][event]") {
    SECTION("Create key press event") {
        auto event = TestHelpers::CreateKeyPress(0x4D, true);
        
        REQUIRE(event.keyCode == 0x4D);
        REQUIRE(event.isDown == true);
        REQUIRE(event.isUp == false);
        REQUIRE(event.timestamp == 12345);
    }
    
    SECTION("Create key release event") {
        auto event = TestHelpers::CreateKeyPress(0x4D, false);
        
        REQUIRE(event.keyCode == 0x4D);
        REQUIRE(event.isDown == false);
        REQUIRE(event.isUp == true);
    }
    
    SECTION("Create events with different key codes") {
        auto event1 = TestHelpers::CreateKeyPress(0x41, true);
        auto event2 = TestHelpers::CreateKeyPress(0x42, true);
        auto event3 = TestHelpers::CreateKeyPress(0x43, true);
        
        REQUIRE(event1.keyCode == 0x41);
        REQUIRE(event2.keyCode == 0x42);
        REQUIRE(event3.keyCode == 0x43);
    }
}

TEST_CASE("Input Consumer Key Binding", "[input][consumer]") {
    Mock::InputConsumer consumer;
    
    SECTION("Bind key with callback") {
        bool callbackCalled = false;
        uint32_t receivedCode = 0;
        
        auto callback = [&callbackCalled, &receivedCode](const Mock::InputEvent& e) {
            callbackCalled = true;
            receivedCode = e.keyCode;
        };
        
        consumer.BindAction(0x4D, callback);
        
        REQUIRE(consumer.bindings.size() == 1);
        REQUIRE(consumer.bindings.find(0x4D) != consumer.bindings.end());
    }
    
    SECTION("Invoke bound callback") {
        bool callbackCalled = false;
        
        auto callback = [&callbackCalled](const Mock::InputEvent& e) {
            callbackCalled = true;
        };
        
        consumer.BindAction(0x4D, callback);
        
        Mock::InputEvent event = TestHelpers::CreateKeyPress(0x4D, true);
        consumer.bindings[0x4D](event);
        
        REQUIRE(callbackCalled == true);
    }
    
    SECTION("Unbind key") {
        auto callback = [](const Mock::InputEvent&) {};
        consumer.BindAction(0x4D, callback);
        
        REQUIRE(consumer.bindings.size() == 1);
        
        consumer.UnbindAction(0x4D);
        
        REQUIRE(consumer.bindings.size() == 0);
        REQUIRE(consumer.bindings.find(0x4D) == consumer.bindings.end());
    }
    
    SECTION("Multiple key bindings") {
        consumer.BindAction(0x4D, [](const Mock::InputEvent&) {});
        consumer.BindAction(0x41, [](const Mock::InputEvent&) {});
        consumer.BindAction(0x42, [](const Mock::InputEvent&) {});
        
        REQUIRE(consumer.bindings.size() == 3);
    }
}

TEST_CASE("Input Manager Processing", "[input][manager]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Get input manager") {
        auto* manager = Mock::InputManager::Get();
        REQUIRE(manager != nullptr);
    }
    
    SECTION("Process input event with callback") {
        auto* manager = Mock::InputManager::Get();
        bool callbackInvoked = false;
        
        manager->RegisterCallback([&callbackInvoked](const Mock::InputEvent& e) {
            callbackInvoked = true;
        });
        
        Mock::InputEvent event = TestHelpers::CreateKeyPress(0x4D, true);
        manager->ProcessEvent(event);
        
        REQUIRE(callbackInvoked == true);
    }
    
    SECTION("Process without callback") {
        auto* manager = Mock::InputManager::Get();
        
        Mock::InputEvent event = TestHelpers::CreateKeyPress(0x4D, true);
        REQUIRE_NOTHROW(manager->ProcessEvent(event));
    }
    
    SECTION("Clear callbacks") {
        auto* manager = Mock::InputManager::Get();
        
        manager->RegisterCallback([](const Mock::InputEvent&) {});
        manager->RegisterCallback([](const Mock::InputEvent&) {});
        
        manager->ClearCallbacks();
        
        Mock::InputEvent event = TestHelpers::CreateKeyPress(0x4D, true);
        manager->ProcessEvent(event);
    }
    
    SECTION("Multiple events processed") {
        auto* manager = Mock::InputManager::Get();
        int callCount = 0;
        
        manager->RegisterCallback([&callCount](const Mock::InputEvent& e) {
            callCount++;
            return;
        });
        
        Mock::InputEvent event1 = TestHelpers::CreateKeyPress(0x4D, true);
        Mock::InputEvent event2 = TestHelpers::CreateKeyPress(0x4D, false);
        Mock::InputEvent event3 = TestHelpers::CreateKeyPress(0x41, true);
        
        manager->ProcessEvent(event1);
        manager->ProcessEvent(event2);
        manager->ProcessEvent(event3);
        
        REQUIRE(callCount == 3);
    }
}

TEST_CASE("M Key Detection", "[input][specific]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Detect M key press (0x4D)") {
        auto* manager = Mock::InputManager::Get();
        bool mKeyPressed = false;
        
        manager->RegisterCallback([&mKeyPressed](const Mock::InputEvent& e) {
            if (e.keyCode == 0x4D && e.isDown) {
                mKeyPressed = true;
            }
        });
        
        Mock::InputEvent mPressEvent = TestHelpers::CreateKeyPress(0x4D, true);
        manager->ProcessEvent(mPressEvent);
        
        REQUIRE(mKeyPressed == true);
    }
    
    SECTION("Ignore non-M keys") {
        auto* manager = Mock::InputManager::Get();
        bool mKeyPressed = false;
        
        manager->RegisterCallback([&mKeyPressed](const Mock::InputEvent& e) {
            if (e.keyCode == 0x4D && e.isDown) {
                mKeyPressed = true;
            }
        });
        
        Mock::InputEvent aPressEvent = TestHelpers::CreateKeyPress(0x41, true);
        manager->ProcessEvent(aPressEvent);
        
        REQUIRE(mKeyPressed == false);
    }
    
    SECTION("M key press and release cycle") {
        auto* manager = Mock::InputManager::Get();
        int pressCount = 0;
        int releaseCount = 0;
        
        manager->RegisterCallback([&pressCount, &releaseCount](const Mock::InputEvent& e) {
            if (e.keyCode == 0x4D) {
                if (e.isDown) {
                    pressCount++;
                } else if (e.isUp) {
                    releaseCount++;
                }
            }
        });
        
        Mock::InputEvent press = TestHelpers::CreateKeyPress(0x4D, true);
        Mock::InputEvent release = TestHelpers::CreateKeyPress(0x4D, false);
        
        manager->ProcessEvent(press);
        manager->ProcessEvent(release);
        
        REQUIRE(pressCount == 1);
        REQUIRE(releaseCount == 1);
    }
}