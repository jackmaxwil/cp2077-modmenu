#include "../CatchSetup.hpp"
#include "../mocks/MockAddressResolver.hpp"

using namespace ModMenu::Addresses;

TEST_CASE("Address Table Definition", "[address][table]") {
    SECTION("Has all required addresses") {
        REQUIRE(g_addressTable.count == 6);
        
        bool foundAsyncSpawnExt = false;
        bool foundAsyncSpawnLocal = false;
        bool foundSpawnExt = false;
        bool foundSpawnLocal = false;
        bool foundFinishAsync = false;
        bool foundInitEngine = false;
        
        for (size_t i = 0; i < g_addressTable.count; ++i) {
            const AddressEntry& entry = g_addressTable.entries[i];
            
            if (entry.hash == HASH_InkWidgetLibrary_AsyncSpawnFromExternal) {
                foundAsyncSpawnExt = true;
            } else if (entry.hash == HASH_InkWidgetLibrary_AsyncSpawnFromLocal) {
                foundAsyncSpawnLocal = true;
            } else if (entry.hash == HASH_InkWidgetLibrary_SpawnFromExternal) {
                foundSpawnExt = true;
            } else if (entry.hash == HASH_InkWidgetLibrary_SpawnFromLocal) {
                foundSpawnLocal = true;
            } else if (entry.hash == HASH_InkSpawner_FinishAsyncSpawn) {
                foundFinishAsync = true;
            } else if (entry.hash == HASH_CBaseEngine_InitEngine) {
                foundInitEngine = true;
            }
        }
        
        REQUIRE(foundAsyncSpawnExt);
        REQUIRE(foundAsyncSpawnLocal);
        REQUIRE(foundSpawnExt);
        REQUIRE(foundSpawnLocal);
        REQUIRE(foundFinishAsync);
        REQUIRE(foundInitEngine);
    }
    
    SECTION("Address entries have names") {
        for (size_t i = 0; i < g_addressTable.count; ++i) {
            const AddressEntry& entry = g_addressTable.entries[i];
            REQUIRE(entry.name != nullptr);
            REQUIRE(entry.hash != 0);
        }
    }
    
    SECTION("Hash values match ArchiveXL") {
        REQUIRE(HASH_InkWidgetLibrary_AsyncSpawnFromExternal == 1396063719);
        REQUIRE(HASH_InkWidgetLibrary_AsyncSpawnFromLocal == 118698863);
        REQUIRE(HASH_InkWidgetLibrary_SpawnFromExternal == 506278179);
        REQUIRE(HASH_InkWidgetLibrary_SpawnFromLocal == 1158555307);
        REQUIRE(HASH_InkSpawner_FinishAsyncSpawn == 2698985195);
        REQUIRE(HASH_CBaseEngine_InitEngine == 3273923080);
    }
}

TEST_CASE("Address Resolution", "[address][resolver]") {
    ClearMockAddresses();
    
    SECTION("Resolve all addresses") {
        bool success = AddressResolver::ResolveAddresses();
        REQUIRE(success);
    }
    
    SECTION("Get address after resolution") {
        AddressResolver::ResolveAddresses();
        
        uintptr_t addr = AddressResolver::GetAddress(HASH_InkWidgetLibrary_SpawnFromLocal);
        REQUIRE(addr != 0);
    }
    
    SECTION("Check if address is resolved") {
        AddressResolver::ResolveAddresses();
        
        REQUIRE(AddressResolver::IsAddressResolved(HASH_InkWidgetLibrary_SpawnFromLocal));
        REQUIRE(AddressResolver::IsAddressResolved(HASH_CBaseEngine_InitEngine));
    }
    
    SECTION("Get non-existent address") {
        AddressResolver::ResolveAddresses();
        
        uintptr_t addr = AddressResolver::GetAddress(0xDEADBEEF);
        REQUIRE(addr == 0);
    }
    
    SECTION("All addresses resolved") {
        AddressResolver::ResolveAddresses();
        
        for (size_t i = 0; i < g_addressTable.count; ++i) {
            const AddressEntry& entry = g_addressTable.entries[i];
            REQUIRE(AddressResolver::IsAddressResolved(entry.hash));
        }
    }
}

TEST_CASE("Address Mock Values", "[address][mock]") {
    ClearMockAddresses();
    
    SECTION("Addresses are sequential mock values") {
        AddressResolver::ResolveAddresses();
        
        uintptr_t addr1 = AddressResolver::GetAddress(HASH_InkWidgetLibrary_AsyncSpawnFromExternal);
        uintptr_t addr2 = AddressResolver::GetAddress(HASH_InkWidgetLibrary_AsyncSpawnFromLocal);
        
        REQUIRE(addr1 == 0x10000);
        REQUIRE(addr2 == 0x11000);
        REQUIRE((addr2 - addr1) == 0x1000);
    }
}

TEST_CASE("Address Clear", "[address][mock]") {
    SECTION("Clear addresses") {
        AddressResolver::ResolveAddresses();
        REQUIRE(AddressResolver::IsAddressResolved(HASH_InkWidgetLibrary_SpawnFromLocal));
        
        ClearMockAddresses();
        
        REQUIRE(AddressResolver::GetAddress(HASH_InkWidgetLibrary_SpawnFromLocal) == 0);
    }
}