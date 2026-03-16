#include "MockRed4.hpp"

namespace Mock {

void* RedClass::Create(CRTTI* rtti) {
    auto instance = new RedClass();
    instance->rtti = rtti;
    instance->thisptr = reinterpret_cast<uint64_t>(instance);
    return instance;
}

Handle Handle::Create(RedClass* ptr) {
    Handle h;
    h.instance = ptr;
    h.refCount = 1;
    return h;
}

Handle Handle::FromPtr(uint64_t ptr) {
    Handle h;
    h.instance = reinterpret_cast<RedClass*>(ptr);
    h.refCount = 1;
    return h;
}

static std::vector<std::unique_ptr<RedClass>> g_redClassStorage;

RedClass* CreateTestRedClass(uint64_t hash) {
    auto rtti = CRTTI::Get(hash);
    if (!rtti) {
        return nullptr;
    }
    auto instance = std::make_unique<RedClass>();
    instance->rtti = rtti;
    instance->thisptr = reinterpret_cast<uint64_t>(instance.get());
    RedClass* ptr = instance.get();
    g_redClassStorage.push_back(std::move(instance));
    return ptr;
}

void ClearMockRedClasses() {
    g_redClassStorage.clear();
}

} // namespace Mock