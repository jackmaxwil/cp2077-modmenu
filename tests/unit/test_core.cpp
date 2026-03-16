#include "../CatchSetup.hpp"
#include "../mocks/MockRED4ext.hpp"
#include "../mocks/MockRed4.hpp"
#include "../mocks/MockInk.hpp"
#include <cstdint>

TEST_CASE("RTTI Registration and Lookup", "[core][rtti]") {
    Mock::ClearMockRTTI();
    
    SECTION("Register RTTI type") {
        uint64_t testHash = 0xDEADBEEF;
        const char* testName = "TestType";
        
        Mock::RegisterRTTI(testHash, testName, 128);
        
        auto* rtti = Mock::CRTTI::Get(testHash);
        REQUIRE(rtti != nullptr);
        REQUIRE(rtti->hash == testHash);
        REQUIRE(std::string(rtti->name) == testName);
        REQUIRE(rtti->size == 128);
    }
    
    SECTION("Multiple RTTI types") {
        Mock::RegisterRTTI(0x11111111, "TypeA", 64);
        Mock::RegisterRTTI(0x22222222, "TypeB", 128);
        Mock::RegisterRTTI(0x33333333, "TypeC", 256);
        
        auto* a = Mock::CRTTI::Get(0x11111111);
        auto* b = Mock::CRTTI::Get(0x22222222);
        auto* c = Mock::CRTTI::Get(0x33333333);
        
        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(c != nullptr);
        REQUIRE(a != b);
        REQUIRE(b != c);
        REQUIRE(a != c);
    }
    
    SECTION("Lookup non-existent type") {
        auto* rtti = Mock::CRTTI::Get(0x0BADF00D);
        REQUIRE(rtti == nullptr);
    }
}

TEST_CASE("RedClass Creation", "[core][red4]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Create RedClass from RTTI") {
        uint64_t testHash = 0x12345678;
        Mock::RegisterRTTI(testHash, "TestClass", 128);
        
        auto* rtti = Mock::CRTTI::Get(testHash);
        REQUIRE(rtti != nullptr);
        
        auto instance = Mock::CreateTestRedClass(testHash);
        REQUIRE(instance != nullptr);
        REQUIRE(instance->rtti == rtti);
        REQUIRE(instance->thisptr != 0);
    }
    
    SECTION("Create with non-existent RTTI") {
        auto instance = Mock::CreateTestRedClass(0x0BADF00D);
        REQUIRE(instance == nullptr);
    }
    
    SECTION("Multiple instances") {
        auto* instance1 = Mock::CreateTestRedClass(0x12345678);
        auto* instance2 = Mock::CreateTestRedClass(0x12345678);
        auto* instance3 = Mock::CreateTestRedClass(0x12345678);
        
        REQUIRE(instance1 != nullptr);
        REQUIRE(instance2 != nullptr);
        REQUIRE(instance3 != nullptr);
        REQUIRE(instance1 != instance2);
        REQUIRE(instance2 != instance3);
    }
}

TEST_CASE("Handle Management", "[core][red4]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Create Handle from pointer") {
        auto* instance = Mock::CreateTestRedClass(0x12345678);
        REQUIRE(instance != nullptr);
        
        auto handle = Mock::Handle::Create(instance);
        REQUIRE(handle.instance == instance);
        REQUIRE(handle.refCount == 1);
    }
    
    SECTION("Create Handle from raw value") {
        uint64_t testPtr = 0xDEADBEEF;
        auto handle = Mock::Handle::FromPtr(testPtr);
        REQUIRE(handle.instance != nullptr);
        REQUIRE(reinterpret_cast<uint64_t>(handle.instance) == testPtr);
    }
}

TEST_CASE("InstancePtr Operations", "[core][red4ext]") {
    SECTION("Create InstancePtr") {
        Mock::InstancePtr ptr(0x12345678);
        REQUIRE(ptr.value == 0x12345678);
    }
    
    SECTION("InstancePtr conversion") {
        Mock::InstancePtr ptr(0xFEDCBA98);
        uint64_t value = static_cast<uint64_t>(ptr);
        REQUIRE(value == 0xFEDCBA98);
    }
    
    SECTION("InstancePtr boolean") {
        Mock::InstancePtr validPtr(0x12345678);
        Mock::InstancePtr nullPtr(0);
        
        REQUIRE(static_cast<bool>(validPtr) == true);
        REQUIRE(static_cast<bool>(nullPtr) == false);
    }
    
    SECTION("InstancePtr comparison") {
        Mock::InstancePtr ptr1(0x12345678);
        Mock::InstancePtr ptr2(0x12345678);
        Mock::InstancePtr ptr3(0x87654321);
        
        REQUIRE(ptr1 == ptr2);
        REQUIRE(ptr1 != ptr3);
    }
}