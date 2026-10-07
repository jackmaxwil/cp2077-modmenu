#include "modmenu_backend.hpp"

#include <RED4ext/RED4ext.hpp>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string_view>

#if defined(__APPLE__)
#include <dlfcn.h>
#include <mach-o/dyld.h>
#endif

namespace
{
std::string ToString(ModMenuString s)
{
    return s.ptr ? std::string(s.ptr) : std::string();
}

std::string_view SkipWs(std::string_view s)
{
    while (!s.empty())
    {
        const char c = s.front();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            s.remove_prefix(1);
        else
            break;
    }
    return s;
}

bool ConsumeChar(std::string_view& s, char c)
{
    s = SkipWs(s);
    if (!s.empty() && s.front() == c)
    {
        s.remove_prefix(1);
        return true;
    }
    return false;
}

bool ParseQuotedString(std::string_view& s, std::string& out)
{
    s = SkipWs(s);
    if (s.empty() || s.front() != '"')
        return false;
    s.remove_prefix(1);
    std::string r;
    while (!s.empty())
    {
        const char c = s.front();
        s.remove_prefix(1);
        if (c == '"')
        {
            out = std::move(r);
            return true;
        }
        if (c == '\\')
        {
            if (s.empty())
                return false;
            const char esc = s.front();
            s.remove_prefix(1);
            switch (esc)
            {
            case '"':
            case '\\':
            case '/':
                r.push_back(esc);
                break;
            case 'n':
                r.push_back('\n');
                break;
            case 'r':
                r.push_back('\r');
                break;
            case 't':
                r.push_back('\t');
                break;
            default:
                // v0.1: minimal escapes
                return false;
            }
        }
        else
        {
            r.push_back(c);
        }
    }
    return false;
}

bool ParseBool(std::string_view& s, bool& out)
{
    s = SkipWs(s);
    if (s.rfind("true", 0) == 0)
    {
        s.remove_prefix(4);
        out = true;
        return true;
    }
    if (s.rfind("false", 0) == 0)
    {
        s.remove_prefix(5);
        out = false;
        return true;
    }
    return false;
}

bool ParseFloat(std::string_view& s, float& out)
{
    s = SkipWs(s);
    if (s.empty())
        return false;
    char* end = nullptr;
    const std::string tmp(s);
    errno = 0;
    const float v = std::strtof(tmp.c_str(), &end);
    if (errno != 0 || end == tmp.c_str())
        return false;
    const std::size_t consumed = static_cast<std::size_t>(end - tmp.c_str());
    s.remove_prefix(consumed);
    out = v;
    return true;
}

bool ExtractObjectAfterKey(std::string_view json, std::string_view key, std::string_view& outObj)
{
    const auto pos = json.find(key);
    if (pos == std::string_view::npos)
        return false;
    auto s = json.substr(pos + key.size());
    const auto brace = s.find('{');
    if (brace == std::string_view::npos)
        return false;
    s.remove_prefix(brace);
    // find matching } (no nested objects expected in our maps)
    int depth = 0;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const char c = s[i];
        if (c == '{')
            depth++;
        else if (c == '}')
        {
            depth--;
            if (depth == 0)
            {
                outObj = s.substr(0, i + 1);
                return true;
            }
        }
    }
    return false;
}

#if defined(__APPLE__)
using ModMenuRegisterFn = bool (*)(const ModMenuApi*);

void ForEachLoadedImage(const std::function<void(const char* path)>& fn)
{
    const auto count = _dyld_image_count();
    for (uint32_t i = 0; i < count; ++i)
    {
        const char* name = _dyld_get_image_name(i);
        if (name && *name)
        {
            fn(name);
        }
    }
}
#endif
} // namespace

namespace ModMenu
{
Backend& Backend::Get()
{
    static Backend s;
    return s;
}

Backend::Backend()
{
    m_api.apiVersion = MODMENU_API_VERSION;
    m_api.RegisterMod = &Backend::Api_RegisterMod;
    m_api.RegisterPage = &Backend::Api_RegisterPage;
    m_api.RegisterToggle = &Backend::Api_RegisterToggle;
    m_api.RegisterSlider = &Backend::Api_RegisterSlider;
    m_api.RegisterButton = &Backend::Api_RegisterButton;
}

void Backend::SetSdk(RED4ext::v1::PluginHandle handle, const RED4ext::v1::Sdk* sdk)
{
    std::scoped_lock _(m_mutex);
    m_handle = handle;
    m_sdk = sdk;
}

const ModMenuApi* Backend::GetApi() const
{
    return &m_api;
}

bool Backend::IsOpen() const
{
    std::scoped_lock _(m_mutex);
    return m_open;
}

void Backend::SetOpen(bool open)
{
    std::scoped_lock _(m_mutex);
    m_open = open;
}

std::size_t Backend::GetModCount() const
{
    std::scoped_lock _(m_mutex);
    return m_mods.size();
}

const Mod* Backend::GetModByIndex(std::size_t idx) const
{
    std::scoped_lock _(m_mutex);
    if (idx >= m_mods.size())
        return nullptr;
    return &m_mods[idx];
}

std::size_t Backend::GetPageCount(const char* modId) const
{
    std::scoped_lock _(m_mutex);
    if (!modId)
        return 0;
    const auto* mod = FindMod(modId);
    return mod ? mod->pages.size() : 0;
}

const Page* Backend::GetPageByIndex(const char* modId, std::size_t idx) const
{
    std::scoped_lock _(m_mutex);
    if (!modId)
        return nullptr;
    const auto* mod = FindMod(modId);
    if (!mod || idx >= mod->pages.size())
        return nullptr;
    return &mod->pages[idx];
}

std::size_t Backend::GetEntryCount(const char* modId, const char* pageId) const
{
    std::scoped_lock _(m_mutex);
    if (!modId || !pageId)
        return 0;
    const auto* mod = FindMod(modId);
    if (!mod)
        return 0;
    const auto* page = FindPage(*mod, pageId);
    return page ? page->entries.size() : 0;
}

const Entry* Backend::GetEntryByIndex(const char* modId, const char* pageId, std::size_t idx) const
{
    std::scoped_lock _(m_mutex);
    if (!modId || !pageId)
        return nullptr;
    const auto* mod = FindMod(modId);
    if (!mod)
        return nullptr;
    const auto* page = FindPage(*mod, pageId);
    if (!page || idx >= page->entries.size())
        return nullptr;
    return &page->entries[idx];
}

const Entry* Backend::GetEntryById(const char* modId, const char* pageId, const char* entryId) const
{
    std::scoped_lock _(m_mutex);
    if (!modId || !pageId || !entryId)
        return nullptr;
    const auto* mod = FindMod(modId);
    if (!mod)
        return nullptr;
    const auto* page = FindPage(*mod, pageId);
    if (!page)
        return nullptr;
    return FindEntry(*page, entryId);
}

bool Backend::SetToggle(const char* modId, const char* pageId, const char* entryId, bool value)
{
    std::scoped_lock _(m_mutex);
    if (!modId || !pageId || !entryId)
        return false;
    auto* mod = FindMod(modId);
    if (!mod)
        return false;
    auto* page = FindPage(*mod, pageId);
    if (!page)
        return false;
    auto* entry = FindEntry(*page, entryId);
    if (!entry || entry->type != MODMENU_ENTRY_TOGGLE)
        return false;

    entry->boolValue = value;

    // Persist (v0.1)
    const std::string modKey(mod->id);
    m_savedBools[modKey][MakeEntryKey(page->id, entry->id)] = value;
    SaveSettingsForMod(modKey);

    if (entry->onToggleChanged)
    {
        if (m_disabledMods.contains(modKey))
            return true;
        ModMenuEntryPath path{
            .modId = {.id = {mod->id.c_str()}},
            .pageId = {.id = {page->id.c_str()}},
            .entryId = {.id = {entry->id.c_str()}},
        };
        try
        {
            const auto t0 = std::chrono::steady_clock::now();
            entry->onToggleChanged(&path, value);
            const auto t1 = std::chrono::steady_clock::now();
            const auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            if (us > 5000) // 5ms budget (very conservative)
            {
                const auto faults = ++m_faultCounts[modKey];
                if (faults >= 3)
                {
                    m_disabledMods[modKey] = "toggle callback exceeded time budget";
                    LogWarn("Disabling ModMenu callbacks for mod '" + modKey + "' (slow callback)");
                }
            }
        }
        catch (...)
        {
            const auto faults = ++m_faultCounts[modKey];
            if (faults >= 1 && !m_disabledMods.contains(modKey))
            {
                m_disabledMods[modKey] = "toggle callback threw exception";
                LogWarn("Disabling ModMenu callbacks for mod '" + modKey + "' (exception)");
            }
        }
    }
    return true;
}

bool Backend::SetSlider(const char* modId, const char* pageId, const char* entryId, float value)
{
    std::scoped_lock _(m_mutex);
    if (!modId || !pageId || !entryId)
        return false;
    auto* mod = FindMod(modId);
    if (!mod)
        return false;
    auto* page = FindPage(*mod, pageId);
    if (!page)
        return false;
    auto* entry = FindEntry(*page, entryId);
    if (!entry || entry->type != MODMENU_ENTRY_SLIDER)
        return false;

    value = std::clamp(value, entry->minValue, entry->maxValue);
    entry->floatValue = value;

    if (entry->onSliderChanged)
    {
        const std::string modKey(mod->id);
        if (m_disabledMods.contains(modKey))
            return true;
        ModMenuEntryPath path{
            .modId = {.id = {mod->id.c_str()}},
            .pageId = {.id = {page->id.c_str()}},
            .entryId = {.id = {entry->id.c_str()}},
        };
        try
        {
            const auto t0 = std::chrono::steady_clock::now();
            entry->onSliderChanged(&path, value);
            const auto t1 = std::chrono::steady_clock::now();
            const auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            if (us > 5000)
            {
                const auto faults = ++m_faultCounts[modKey];
                if (faults >= 3)
                {
                    m_disabledMods[modKey] = "slider callback exceeded time budget";
                    LogWarn("Disabling ModMenu callbacks for mod '" + modKey + "' (slow callback)");
                }
            }
        }
        catch (...)
        {
            const auto faults = ++m_faultCounts[modKey];
            if (faults >= 1 && !m_disabledMods.contains(modKey))
            {
                m_disabledMods[modKey] = "slider callback threw exception";
                LogWarn("Disabling ModMenu callbacks for mod '" + modKey + "' (exception)");
            }
        }
    }

    // Persist (v0.1)
    {
        const std::string modKey(mod->id);
        m_savedFloats[modKey][MakeEntryKey(page->id, entry->id)] = value;
        SaveSettingsForMod(modKey);
    }
    return true;
}

bool Backend::PressButton(const char* modId, const char* pageId, const char* entryId)
{
    std::scoped_lock _(m_mutex);
    if (!modId || !pageId || !entryId)
        return false;
    auto* mod = FindMod(modId);
    if (!mod)
        return false;
    auto* page = FindPage(*mod, pageId);
    if (!page)
        return false;
    auto* entry = FindEntry(*page, entryId);
    if (!entry || entry->type != MODMENU_ENTRY_BUTTON)
        return false;

    if (entry->onButtonPressed)
    {
        const std::string modKey(mod->id);
        if (m_disabledMods.contains(modKey))
            return true;
        ModMenuEntryPath path{
            .modId = {.id = {mod->id.c_str()}},
            .pageId = {.id = {page->id.c_str()}},
            .entryId = {.id = {entry->id.c_str()}},
        };
        try
        {
            const auto t0 = std::chrono::steady_clock::now();
            entry->onButtonPressed(&path);
            const auto t1 = std::chrono::steady_clock::now();
            const auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            if (us > 5000)
            {
                const auto faults = ++m_faultCounts[modKey];
                if (faults >= 3)
                {
                    m_disabledMods[modKey] = "button callback exceeded time budget";
                    LogWarn("Disabling ModMenu callbacks for mod '" + modKey + "' (slow callback)");
                }
            }
        }
        catch (...)
        {
            const auto faults = ++m_faultCounts[modKey];
            if (faults >= 1 && !m_disabledMods.contains(modKey))
            {
                m_disabledMods[modKey] = "button callback threw exception";
                LogWarn("Disabling ModMenu callbacks for mod '" + modKey + "' (exception)");
            }
        }
    }
    return true;
}

void Backend::DemoToggleChanged(bool value)
{
    LogInfo(std::string("Demo toggle changed: ") + (value ? "on" : "off"));
}

std::filesystem::path Backend::GetPluginDir() const
{
#if defined(__APPLE__)
    Dl_info info{};
    if (dladdr(reinterpret_cast<const void*>(&Backend::Get), &info) && info.dli_fname)
    {
        return std::filesystem::path(info.dli_fname).parent_path();
    }
#endif
    return {};
}

std::filesystem::path Backend::GetRed4extRoot() const
{
    auto dir = GetPluginDir();
    // Walk upwards looking for .../red4ext/plugins/ModMenu/
    for (int i = 0; i < 10 && !dir.empty(); ++i)
    {
        if (dir.filename() == "red4ext")
            return dir;
        const auto candidate = dir / "red4ext";
        if (std::filesystem::exists(candidate) && std::filesystem::is_directory(candidate))
            return candidate;
        dir = dir.parent_path();
    }
    return {};
}

std::string Backend::GetRed4extLogTail(std::size_t maxBytes) const
{
    const auto root = GetRed4extRoot();
    if (root.empty())
        return {};

    const auto logDir = root / "logs";
    if (!std::filesystem::exists(logDir))
        return {};

    // Pick the newest red4ext-*.log file.
    std::filesystem::path newest;
    std::filesystem::file_time_type newestTime{};
    bool have = false;

    for (const auto& entry : std::filesystem::directory_iterator(logDir))
    {
        if (!entry.is_regular_file())
            continue;
        const auto p = entry.path();
        const auto name = p.filename().string();
        if (name.rfind("red4ext-", 0) != 0 || p.extension() != ".log")
            continue;
        const auto t = entry.last_write_time();
        if (!have || t > newestTime)
        {
            newestTime = t;
            newest = p;
            have = true;
        }
    }

    if (!have)
        return {};
    return ReadTail(newest, maxBytes);
}

std::string Backend::GetPluginLogTail(const char* modId, std::size_t maxBytes) const
{
    if (!modId || !*modId)
        return {};
    const auto root = GetRed4extRoot();
    if (root.empty())
        return {};
    const auto pluginDir = root / "plugins" / modId;
    if (!std::filesystem::exists(pluginDir))
        return {};

    std::filesystem::path newest;
    std::filesystem::file_time_type newestTime{};
    bool have = false;
    for (const auto& entry : std::filesystem::directory_iterator(pluginDir))
    {
        if (!entry.is_regular_file())
            continue;
        const auto p = entry.path();
        if (p.extension() != ".log")
            continue;
        const auto t = entry.last_write_time();
        if (!have || t > newestTime)
        {
            newestTime = t;
            newest = p;
            have = true;
        }
    }
    if (!have)
        return {};
    return ReadTail(newest, maxBytes);
}

std::string Backend::ScanIncompatiblePlugins() const
{
    const auto root = GetRed4extRoot();
    if (root.empty())
        return {};

    const auto pluginsDir = root / "plugins";
    if (!std::filesystem::exists(pluginsDir))
        return {};

    std::ostringstream out;
    bool first = true;

    for (const auto& entry : std::filesystem::directory_iterator(pluginsDir))
    {
        if (!entry.is_directory())
            continue;

        const auto dir = entry.path();
        bool hasDll = false;
        bool hasDylib = false;
        for (const auto& f : std::filesystem::directory_iterator(dir))
        {
            if (!f.is_regular_file())
                continue;
            const auto ext = f.path().extension().string();
            if (ext == ".dll")
                hasDll = true;
            else if (ext == ".dylib")
                hasDylib = true;
        }

        if (hasDll && !hasDylib)
        {
            out << (first ? "" : "\n");
            first = false;
            out << dir.filename().string();
        }
    }

    return out.str();
}

std::string Backend::GenerateReportBundle(std::size_t logMaxBytes) const
{
    const auto root = GetRed4extRoot();
    if (root.empty())
        return {};

    const auto logDir = root / "logs";
    std::error_code ec;
    std::filesystem::create_directories(logDir, ec);

    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto ts = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    const auto outDir = logDir / ("modmenu-report-" + std::to_string(ts));
    std::filesystem::create_directories(outDir, ec);

    // Dump a simple menu snapshot (no full JSON library; stable enough for debugging).
    {
        std::ostringstream json;
        json << "{\n";
        json << "  \"version\": 1,\n";
        json << "  \"mods\": [\n";
        bool firstMod = true;
        {
            std::scoped_lock _(m_mutex);
            for (const auto& mod : m_mods)
            {
                json << (firstMod ? "" : ",\n");
                firstMod = false;
                json << "    {\n";
                json << "      \"id\": \"" << EscapeJson(mod.id) << "\",\n";
                json << "      \"name\": \"" << EscapeJson(mod.name) << "\",\n";
                json << "      \"version\": \"" << EscapeJson(mod.version) << "\",\n";
                json << "      \"pages\": [\n";
                bool firstPage = true;
                for (const auto& page : mod.pages)
                {
                    json << (firstPage ? "" : ",\n");
                    firstPage = false;
                    json << "        {\n";
                    json << "          \"id\": \"" << EscapeJson(page.id) << "\",\n";
                    json << "          \"title\": \"" << EscapeJson(page.title) << "\",\n";
                    json << "          \"entries\": [\n";
                    bool firstEntry = true;
                    for (const auto& e : page.entries)
                    {
                        json << (firstEntry ? "" : ",\n");
                        firstEntry = false;
                        json << "            {\n";
                        json << "              \"id\": \"" << EscapeJson(e.id) << "\",\n";
                        json << "              \"title\": \"" << EscapeJson(e.title) << "\",\n";
                        json << "              \"type\": " << static_cast<std::uint32_t>(e.type) << ",\n";
                        json << "              \"boolValue\": " << (e.boolValue ? "true" : "false") << ",\n";
                        json << "              \"floatValue\": " << std::setprecision(7) << e.floatValue << "\n";
                        json << "            }";
                    }
                    json << "\n          ]\n";
                    json << "        }";
                }
                json << "\n      ]\n";
                json << "    }";
            }
        }
        json << "\n  ]\n";
        json << "}\n";

        std::ofstream f(outDir / "menu.json", std::ios::binary | std::ios::trunc);
        const auto s = json.str();
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
    }

    // Dump log tails
    {
        std::ofstream f(outDir / "red4ext.log.tail.txt", std::ios::binary | std::ios::trunc);
        const auto s = GetRed4extLogTail(logMaxBytes);
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
    }
    {
        std::ofstream f(outDir / "incompatible_plugins.txt", std::ios::binary | std::ios::trunc);
        const auto s = ScanIncompatiblePlugins();
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
    }

    return outDir.string();
}

void Backend::LoadAllSettings()
{
    std::scoped_lock _(m_mutex);
    for (const auto& mod : m_mods)
    {
        LoadSettingsForMod(mod.id);
    }
}

void Backend::SaveAllSettings() const
{
    std::scoped_lock _(m_mutex);
    for (const auto& mod : m_mods)
    {
        SaveSettingsForMod(mod.id);
    }
}

void Backend::LoadSettingsForMod(const std::string& modId)
{
    const auto settingsDir = GetPluginDir() / "settings";
    const auto path = settingsDir / (modId + ".json");
    if (!std::filesystem::exists(path))
        return;

    std::ifstream f(path, std::ios::binary);
    if (!f)
        return;
    std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    std::string_view json(contents);

    std::string_view boolObj;
    if (ExtractObjectAfterKey(json, "\"bool\"", boolObj))
    {
        std::string_view s = boolObj;
        if (ConsumeChar(s, '{'))
        {
            while (true)
            {
                s = SkipWs(s);
                if (ConsumeChar(s, '}'))
                    break;
                if (ConsumeChar(s, ','))
                    continue;
                std::string k;
                if (!ParseQuotedString(s, k))
                    break;
                if (!ConsumeChar(s, ':'))
                    break;
                bool v = false;
                if (!ParseBool(s, v))
                    break;
                m_savedBools[modId][k] = v;
            }
        }
    }

    std::string_view floatObj;
    if (ExtractObjectAfterKey(json, "\"float\"", floatObj))
    {
        std::string_view s = floatObj;
        if (ConsumeChar(s, '{'))
        {
            while (true)
            {
                s = SkipWs(s);
                if (ConsumeChar(s, '}'))
                    break;
                if (ConsumeChar(s, ','))
                    continue;
                std::string k;
                if (!ParseQuotedString(s, k))
                    break;
                if (!ConsumeChar(s, ':'))
                    break;
                float v = 0.0f;
                if (!ParseFloat(s, v))
                    break;
                m_savedFloats[modId][k] = v;
            }
        }
    }
}

void Backend::SaveSettingsForMod(const std::string& modId) const
{
    const auto settingsDir = GetPluginDir() / "settings";
    std::error_code ec;
    std::filesystem::create_directories(settingsDir, ec);

    const auto path = settingsDir / (modId + ".json");
    const auto tmp = settingsDir / (modId + ".json.tmp");

    auto itB = m_savedBools.find(modId);
    auto itF = m_savedFloats.find(modId);

    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"bool\": {\n";
    bool first = true;
    if (itB != m_savedBools.end())
    {
        for (const auto& [k, v] : itB->second)
        {
            out << (first ? "" : ",\n");
            first = false;
            out << "    \"" << EscapeJson(k) << "\": " << (v ? "true" : "false");
        }
    }
    out << "\n  },\n";
    out << "  \"float\": {\n";
    first = true;
    if (itF != m_savedFloats.end())
    {
        for (const auto& [k, v] : itF->second)
        {
            out << (first ? "" : ",\n");
            first = false;
            out << "    \"" << EscapeJson(k) << "\": " << std::setprecision(7) << v;
        }
    }
    out << "\n  }\n";
    out << "}\n";

    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f)
            return;
        const auto s = out.str();
        f.write(s.data(), static_cast<std::streamsize>(s.size()));
    }
    std::filesystem::rename(tmp, path, ec);
    if (ec)
    {
        // Best-effort cleanup
        std::filesystem::remove(tmp, ec);
    }
}

void Backend::DiscoverAndRegisterExternal()
{
#if defined(__APPLE__)
    // Best-effort: iterate loaded images and call exported ModMenu_Register(const ModMenuApi*).
    ForEachLoadedImage([&](const char* path) {
        if (!path || !*path)
            return;

        // Skip ourselves.
        if (std::strstr(path, "/ModMenu.dylib") != nullptr)
            return;

        void* handle = dlopen(path, RTLD_LAZY | RTLD_NOLOAD);
        if (!handle)
            return;

        auto* fn = reinterpret_cast<ModMenuRegisterFn>(dlsym(handle, "ModMenu_Register"));
        if (!fn)
            return;

        try
        {
            fn(GetApi());
        }
        catch (...)
        {
        }
    });
#endif
}

bool Backend::Api_RegisterMod(const ModMenuModInfo* mod)
{
    if (!mod || !mod->modId.ptr || !*mod->modId.ptr)
        return false;
    auto& self = Backend::Get();
    std::scoped_lock _(self.m_mutex);

    const std::string id = ToString({mod->modId});
    if (id.empty())
        return false;

    if (self.FindMod(id))
        return true;

    self.LoadSettingsForMod(id);

    Mod m{};
    m.id = id;
    m.name = ToString(mod->name);
    if (m.name.empty())
        m.name = m.id;
    m.author = ToString(mod->author);
    m.version = ToString(mod->version);
    m.capabilityFlags = mod->capabilityFlags;
    self.m_mods.emplace_back(std::move(m));
    return true;
}

bool Backend::Api_RegisterPage(const char* modId, const ModMenuPageInfo* page)
{
    if (!modId || !page || !page->pageId.ptr || !*page->pageId.ptr)
        return false;
    auto& self = Backend::Get();
    std::scoped_lock _(self.m_mutex);

    auto* mod = self.FindMod(modId);
    if (!mod)
        return false;

    const std::string pageId = ToString({page->pageId});
    if (pageId.empty())
        return false;

    if (self.FindPage(*mod, pageId))
        return true;

    Page p{};
    p.id = pageId;
    p.title = ToString({page->title});
    if (p.title.empty())
        p.title = p.id;
    mod->pages.emplace_back(std::move(p));
    return true;
}

bool Backend::Api_RegisterToggle(const char* modId, const char* pageId, const ModMenuToggleInfo* toggle)
{
    if (!modId || !pageId || !toggle || !toggle->entryId.ptr || !*toggle->entryId.ptr)
        return false;
    auto& self = Backend::Get();
    std::scoped_lock _(self.m_mutex);
    auto* mod = self.FindMod(modId);
    if (!mod)
        return false;
    auto* page = self.FindPage(*mod, pageId);
    if (!page)
        return false;

    Entry e{};
    e.type = MODMENU_ENTRY_TOGGLE;
    e.id = ToString({toggle->entryId});
    e.title = ToString({toggle->title});
    if (e.title.empty())
        e.title = e.id;
    e.boolValue = toggle->defaultValue;
    {
        const auto itM = self.m_savedBools.find(mod->id);
        if (itM != self.m_savedBools.end())
        {
            const auto k = MakeEntryKey(page->id, e.id);
            const auto it = itM->second.find(k);
            if (it != itM->second.end())
                e.boolValue = it->second;
        }
    }
    e.onToggleChanged = toggle->onChanged;
    page->entries.emplace_back(std::move(e));
    return true;
}

bool Backend::Api_RegisterSlider(const char* modId, const char* pageId, const ModMenuSliderInfo* slider)
{
    if (!modId || !pageId || !slider || !slider->entryId.ptr || !*slider->entryId.ptr)
        return false;
    auto& self = Backend::Get();
    std::scoped_lock _(self.m_mutex);
    auto* mod = self.FindMod(modId);
    if (!mod)
        return false;
    auto* page = self.FindPage(*mod, pageId);
    if (!page)
        return false;

    Entry e{};
    e.type = MODMENU_ENTRY_SLIDER;
    e.id = ToString({slider->entryId});
    e.title = ToString({slider->title});
    if (e.title.empty())
        e.title = e.id;
    e.minValue = slider->minValue;
    e.maxValue = slider->maxValue;
    e.step = slider->step;
    e.floatValue = slider->defaultValue;
    {
        const auto itM = self.m_savedFloats.find(mod->id);
        if (itM != self.m_savedFloats.end())
        {
            const auto k = MakeEntryKey(page->id, e.id);
            const auto it = itM->second.find(k);
            if (it != itM->second.end())
                e.floatValue = it->second;
        }
    }
    e.onSliderChanged = slider->onChanged;
    page->entries.emplace_back(std::move(e));
    return true;
}

bool Backend::Api_RegisterButton(const char* modId, const char* pageId, const ModMenuButtonInfo* button)
{
    if (!modId || !pageId || !button || !button->entryId.ptr || !*button->entryId.ptr)
        return false;
    auto& self = Backend::Get();
    std::scoped_lock _(self.m_mutex);
    auto* mod = self.FindMod(modId);
    if (!mod)
        return false;
    auto* page = self.FindPage(*mod, pageId);
    if (!page)
        return false;

    Entry e{};
    e.type = MODMENU_ENTRY_BUTTON;
    e.id = ToString({button->entryId});
    e.title = ToString({button->title});
    if (e.title.empty())
        e.title = e.id;
    e.onButtonPressed = button->onPressed;
    page->entries.emplace_back(std::move(e));
    return true;
}

const Mod* Backend::FindMod(std::string_view id) const
{
    for (const auto& m : m_mods)
    {
        if (m.id == id)
            return &m;
    }
    return nullptr;
}

Mod* Backend::FindMod(std::string_view id)
{
    for (auto& m : m_mods)
    {
        if (m.id == id)
            return &m;
    }
    return nullptr;
}

Page* Backend::FindPage(Mod& mod, std::string_view pageId)
{
    for (auto& p : mod.pages)
    {
        if (p.id == pageId)
            return &p;
    }
    return nullptr;
}

const Page* Backend::FindPage(const Mod& mod, std::string_view pageId) const
{
    for (const auto& p : mod.pages)
    {
        if (p.id == pageId)
            return &p;
    }
    return nullptr;
}

Entry* Backend::FindEntry(Page& page, std::string_view entryId)
{
    for (auto& e : page.entries)
    {
        if (e.id == entryId)
            return &e;
    }
    return nullptr;
}

const Entry* Backend::FindEntry(const Page& page, std::string_view entryId) const
{
    for (const auto& e : page.entries)
    {
        if (e.id == entryId)
            return &e;
    }
    return nullptr;
}

std::string Backend::ReadTail(const std::filesystem::path& path, std::size_t maxBytes)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
        return {};

    f.seekg(0, std::ios::end);
    const auto size = static_cast<std::size_t>(f.tellg());
    const auto start = (size > maxBytes) ? (size - maxBytes) : 0;
    f.seekg(static_cast<std::streamoff>(start), std::ios::beg);

    std::string buf;
    buf.resize(size - start);
    f.read(buf.data(), static_cast<std::streamsize>(buf.size()));
    return buf;
}

std::string Backend::EscapeJson(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (const char c : s)
    {
        switch (c)
        {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out.push_back(c);
            break;
        }
    }
    return out;
}

std::string Backend::MakeEntryKey(std::string_view pageId, std::string_view entryId)
{
    std::string k;
    k.reserve(pageId.size() + 1 + entryId.size());
    k.append(pageId);
    k.push_back('/');
    k.append(entryId);
    return k;
}

void Backend::LogInfo(const std::string& msg) const
{
    if (m_sdk && m_sdk->logger && m_sdk->logger->InfoF)
        m_sdk->logger->InfoF(m_handle, "[ModMenu] %s", msg.c_str());
}

void Backend::LogWarn(const std::string& msg) const
{
    if (m_sdk && m_sdk->logger && m_sdk->logger->WarnF)
        m_sdk->logger->WarnF(m_handle, "[ModMenu] %s", msg.c_str());
}
} // namespace ModMenu

extern "C" const ModMenuApi* ModMenu_GetApi(void)
{
    return ModMenu::Backend::Get().GetApi();
}

