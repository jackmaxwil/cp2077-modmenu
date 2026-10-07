#pragma once

#include <modmenu/modmenu_api.h>

#include <RED4ext/RED4ext.hpp>

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ModMenu
{
struct Entry
{
    ModMenuEntryType type{};
    std::string id;
    std::string title;

    // Value storage (simple v0.1)
    bool boolValue{false};
    float floatValue{0.0f};
    float minValue{0.0f};
    float maxValue{1.0f};
    float step{0.1f};

    // Callbacks (invoked by backend when UI updates)
    ModMenuOnToggleChangedFn onToggleChanged{nullptr};
    ModMenuOnSliderChangedFn onSliderChanged{nullptr};
    ModMenuOnButtonPressedFn onButtonPressed{nullptr};
};

struct Page
{
    std::string id;
    std::string title;
    std::vector<Entry> entries;
};

struct Mod
{
    std::string id;
    std::string name;
    std::string author;
    std::string version;
    std::uint32_t capabilityFlags{0};
    std::vector<Page> pages;
};

class Backend
{
public:
    static Backend& Get();

    void SetSdk(RED4ext::v1::PluginHandle handle, const RED4ext::v1::Sdk* sdk);

    const ModMenuApi* GetApi() const;

    // State
    bool IsOpen() const;
    void SetOpen(bool open);

    // Query for UI (index-based)
    std::size_t GetModCount() const;
    const Mod* GetModByIndex(std::size_t idx) const;
    std::size_t GetPageCount(const char* modId) const;
    const Page* GetPageByIndex(const char* modId, std::size_t idx) const;
    std::size_t GetEntryCount(const char* modId, const char* pageId) const;
    const Entry* GetEntryByIndex(const char* modId, const char* pageId, std::size_t idx) const;
    const Entry* GetEntryById(const char* modId, const char* pageId, const char* entryId) const;

    // Value access
    bool SetToggle(const char* modId, const char* pageId, const char* entryId, bool value);
    bool SetSlider(const char* modId, const char* pageId, const char* entryId, float value);
    bool PressButton(const char* modId, const char* pageId, const char* entryId);
    void DemoToggleChanged(bool value);

    // Log tail
    std::string GetRed4extLogTail(std::size_t maxBytes) const;
    std::string GetPluginLogTail(const char* modId, std::size_t maxBytes) const;

    // Utility / tooling
    std::string ScanIncompatiblePlugins() const; // newline-separated plugin folder names
    std::string GenerateReportBundle(std::size_t logMaxBytes) const; // returns directory path (string)

    // Settings persistence
    void LoadAllSettings();
    void SaveAllSettings() const;
    void LoadSettingsForMod(const std::string& modId);
    void SaveSettingsForMod(const std::string& modId) const;

    // Best-effort discovery: find other plugins exporting ModMenu_Register and call it.
    void DiscoverAndRegisterExternal();

    // Resolve filesystem locations relative to our own plugin directory.
    std::filesystem::path GetPluginDir() const;
    std::filesystem::path GetRed4extRoot() const;

private:
    Backend();

    static bool Api_RegisterMod(const ModMenuModInfo* mod);
    static bool Api_RegisterPage(const char* modId, const ModMenuPageInfo* page);
    static bool Api_RegisterToggle(const char* modId, const char* pageId, const ModMenuToggleInfo* toggle);
    static bool Api_RegisterSlider(const char* modId, const char* pageId, const ModMenuSliderInfo* slider);
    static bool Api_RegisterButton(const char* modId, const char* pageId, const ModMenuButtonInfo* button);

    const Mod* FindMod(std::string_view id) const;
    Mod* FindMod(std::string_view id);
    Page* FindPage(Mod& mod, std::string_view pageId);
    const Page* FindPage(const Mod& mod, std::string_view pageId) const;
    Entry* FindEntry(Page& page, std::string_view entryId);
    const Entry* FindEntry(const Page& page, std::string_view entryId) const;

    static std::string ReadTail(const std::filesystem::path& path, std::size_t maxBytes);
    static std::string EscapeJson(std::string_view s);
    static std::string MakeEntryKey(std::string_view pageId, std::string_view entryId);

    void LogInfo(const std::string& msg) const;
    void LogWarn(const std::string& msg) const;

private:
    mutable std::mutex m_mutex;

    RED4ext::v1::PluginHandle m_handle{};
    const RED4ext::v1::Sdk* m_sdk{nullptr};

    bool m_open{false};

    std::vector<Mod> m_mods;

    ModMenuApi m_api{};

    // v0.1 persistence: per-mod maps keyed by "pageId/entryId"
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> m_savedBools;
    std::unordered_map<std::string, std::unordered_map<std::string, float>> m_savedFloats;

    // Safety: disable misbehaving mods (callbacks)
    std::unordered_map<std::string, std::uint32_t> m_faultCounts;
    std::unordered_map<std::string, std::string> m_disabledMods;
};
} // namespace ModMenu

