#include "MockRED4ext.hpp"

namespace Mock {

static std::unordered_map<uint64_t, CRTTI*> g_rttiRegistry;
static std::vector<std::unique_ptr<CRTTI>> g_rttiStorage;

CRTTI* CRTTI::Get(uint64_t hash) {
    auto it = g_rttiRegistry.find(hash);
    if (it != g_rttiRegistry.end()) {
        return it->second;
    }
    return nullptr;
}

void RegisterRTTI(uint64_t hash, const char* name, size_t size) {
    auto rtti = std::make_unique<CRTTI>();
    rtti->hash = hash;
    rtti->name = name;
    rtti->size = size;
    g_rttiStorage.push_back(std::move(rtti));
    g_rttiRegistry[hash] = g_rttiStorage.back().get();
}

void ClearMockRTTI() {
    g_rttiRegistry.clear();
    g_rttiStorage.clear();
}

} // namespace Mock