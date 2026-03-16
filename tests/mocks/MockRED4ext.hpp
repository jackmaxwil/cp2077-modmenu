#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <string>
#include <memory>

namespace Mock {

struct InstancePtr {
    uint64_t value = 0;
    
    InstancePtr() = default;
    explicit InstancePtr(uint64_t v) : value(v) {}
    explicit operator uint64_t() const { return value; }
    explicit operator bool() const { return value != 0; }
    bool operator==(const InstancePtr& other) const { return value == other.value; }
    bool operator!=(const InstancePtr& other) const { return value != other.value; }
};

struct CRTTI {
    static CRTTI* Get(uint64_t hash);
    
    uint64_t hash = 0;
    const char* name = nullptr;
    size_t size = 0;
};

void RegisterRTTI(uint64_t hash, const char* name, size_t size);
void ClearMockRTTI();

} // namespace Mock