#pragma once

#include "MockRED4ext.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace Mock {

struct RedClass;
struct ScriptGameInstance;

struct RedClass {
    static void* Create(CRTTI* rtti);
    
    CRTTI* rtti = nullptr;
    uint64_t thisptr = 0;
};

struct Handle {
    static Handle Create(RedClass* ptr);
    static Handle FromPtr(uint64_t ptr);
    
    RedClass* instance = nullptr;
    uint64_t refCount = 0;
    
    explicit operator uint64_t() const { return reinterpret_cast<uint64_t>(instance); }
    explicit operator bool() const { return instance != nullptr; }
};

struct ResourceAsyncReference {
    uint64_t depotPath = 0;
    
    bool IsValid() const { return depotPath != 0; }
};

RedClass* CreateTestRedClass(uint64_t hash);
void ClearMockRedClasses();

} // namespace Mock