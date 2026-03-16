#pragma once

#include <cstdint>
#include <cstddef>

namespace ModMenu::Addresses {

typedef uint32_t AddressHash;

struct AddressEntry {
    AddressHash hash;
    const char* name;
};

struct AddressTable {
    size_t count;
    const AddressEntry* entries;
};

extern AddressEntry g_addressEntries[];
extern AddressTable g_addressTable;

class AddressResolver {
public:
    static bool ResolveAddresses();
    static uintptr_t GetAddress(AddressHash hash);
    static bool IsAddressResolved(AddressHash hash);
    static void Reset();
    
private:
    static bool s_initialized;
};

} // namespace ModMenu::Addresses

constexpr ModMenu::Addresses::AddressHash HASH_InkWidgetLibrary_AsyncSpawnFromExternal = 1396063719;
constexpr ModMenu::Addresses::AddressHash HASH_InkWidgetLibrary_AsyncSpawnFromLocal = 118698863;
constexpr ModMenu::Addresses::AddressHash HASH_InkWidgetLibrary_SpawnFromExternal = 506278179;
constexpr ModMenu::Addresses::AddressHash HASH_InkWidgetLibrary_SpawnFromLocal = 1158555307;
constexpr ModMenu::Addresses::AddressHash HASH_InkSpawner_FinishAsyncSpawn = 2698985195;
constexpr ModMenu::Addresses::AddressHash HASH_CBaseEngine_InitEngine = 3273923080;

void ClearMockAddresses();