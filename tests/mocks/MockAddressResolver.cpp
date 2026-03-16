#include "MockAddressResolver.hpp"
#include <unordered_map>

namespace ModMenu::Addresses {

static std::unordered_map<AddressHash, uintptr_t> g_mockAddressMap;
bool AddressResolver::s_initialized = false;

AddressEntry g_addressEntries[] = {
    {HASH_InkWidgetLibrary_AsyncSpawnFromExternal, "InkWidgetLibrary::AsyncSpawnFromExternal"},
    {HASH_InkWidgetLibrary_AsyncSpawnFromLocal, "InkWidgetLibrary::AsyncSpawnFromLocal"},
    {HASH_InkWidgetLibrary_SpawnFromExternal, "InkWidgetLibrary::SpawnFromExternal"},
    {HASH_InkWidgetLibrary_SpawnFromLocal, "InkWidgetLibrary::SpawnFromLocal"},
    {HASH_InkSpawner_FinishAsyncSpawn, "InkSpawner::FinishAsyncSpawn"},
    {HASH_CBaseEngine_InitEngine, "CBaseEngine::InitEngine"}
};

AddressTable g_addressTable = {
    sizeof(g_addressEntries) / sizeof(AddressEntry),
    g_addressEntries
};

bool AddressResolver::ResolveAddresses() {
    if (s_initialized) {
        return true;
    }
    
    g_mockAddressMap.clear();
    
    for (size_t i = 0; i < g_addressTable.count; ++i) {
        const AddressEntry& entry = g_addressTable.entries[i];
        uintptr_t mockAddr = 0x10000 + (i * 0x1000);
        g_mockAddressMap[entry.hash] = mockAddr;
    }
    
    s_initialized = true;
    return true;
}

uintptr_t AddressResolver::GetAddress(AddressHash hash) {
    auto it = g_mockAddressMap.find(hash);
    if (it != g_mockAddressMap.end()) {
        return it->second;
    }
    return 0;
}

bool AddressResolver::IsAddressResolved(AddressHash hash) {
    return GetAddress(hash) != 0;
}

void AddressResolver::Reset() {
    g_mockAddressMap.clear();
    s_initialized = false;
}

} // namespace ModMenu::Addresses

void ClearMockAddresses() {
    ModMenu::Addresses::AddressResolver::Reset();
}