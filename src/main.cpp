#include "modmenu_backend.hpp"

#include <RED4ext/RED4ext.hpp>

#include <RED4ext/CName.hpp>
#include <RED4ext/CString.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/Scripting/IScriptable.hpp>

// ============================================================================
// Native REDscript bridge functions
// ============================================================================

// Note: For v0.1 we keep the bridge minimal and string-based. REDscript side can
// call these globals to populate UI and set values.

static void ModMenu_IsOpen_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aOut, std::int64_t)
{
    aFrame->code++; // ParamEnd
    if (aOut)
        *aOut = ModMenu::Backend::Get().IsOpen();
}

static void ModMenu_SetOpen_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aOut, std::int64_t)
{
    bool open = false;
    RED4ext::GetParameter(aFrame, &open);
    aFrame->code++; // ParamEnd
    ModMenu::Backend::Get().SetOpen(open);
    if (aOut)
        *aOut = true;
}

static void ModMenu_GetRed4extLogTail_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                                        std::int64_t)
{
    std::int32_t maxBytes = 8192;
    RED4ext::GetParameter(aFrame, &maxBytes);
    aFrame->code++; // ParamEnd
    if (!aOut)
        return;
    const auto s = ModMenu::Backend::Get().GetRed4extLogTail(static_cast<std::size_t>(std::max(0, maxBytes)));
    *aOut = RED4ext::CString(s.c_str());
}

static void ModMenu_GetPluginLogTail_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                                        std::int64_t)
{
    RED4ext::CString modId;
    std::int32_t maxBytes = 8192;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &maxBytes);
    aFrame->code++; // ParamEnd
    if (!aOut)
        return;
    const auto s = ModMenu::Backend::Get().GetPluginLogTail(modId.c_str(), static_cast<std::size_t>(std::max(0, maxBytes)));
    *aOut = RED4ext::CString(s.c_str());
}

static void ModMenu_GetModCount_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, std::int32_t* aOut, std::int64_t)
{
    aFrame->code++; // ParamEnd
    if (aOut)
        *aOut = static_cast<std::int32_t>(ModMenu::Backend::Get().GetModCount());
}

static void ModMenu_GetModId_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, std::int64_t)
{
    std::int32_t idx = 0;
    RED4ext::GetParameter(aFrame, &idx);
    aFrame->code++; // ParamEnd
    if (!aOut || idx < 0)
        return;
    const auto* mod = ModMenu::Backend::Get().GetModByIndex(static_cast<std::size_t>(idx));
    *aOut = mod ? RED4ext::CString(mod->id.c_str()) : RED4ext::CString("");
}

static void ModMenu_GetModName_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, std::int64_t)
{
    std::int32_t idx = 0;
    RED4ext::GetParameter(aFrame, &idx);
    aFrame->code++; // ParamEnd
    if (!aOut || idx < 0)
        return;
    const auto* mod = ModMenu::Backend::Get().GetModByIndex(static_cast<std::size_t>(idx));
    *aOut = mod ? RED4ext::CString(mod->name.c_str()) : RED4ext::CString("");
}

static void ModMenu_GetPageCount_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, std::int32_t* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::GetParameter(aFrame, &modId);
    aFrame->code++; // ParamEnd
    if (!aOut)
        return;
    *aOut = static_cast<std::int32_t>(ModMenu::Backend::Get().GetPageCount(modId.c_str()));
}

static void ModMenu_GetPageId_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, std::int64_t)
{
    RED4ext::CString modId;
    std::int32_t idx = 0;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &idx);
    aFrame->code++; // ParamEnd
    if (!aOut || idx < 0)
        return;
    const auto* page = ModMenu::Backend::Get().GetPageByIndex(modId.c_str(), static_cast<std::size_t>(idx));
    *aOut = page ? RED4ext::CString(page->id.c_str()) : RED4ext::CString("");
}

static void ModMenu_GetPageTitle_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                                   std::int64_t)
{
    RED4ext::CString modId;
    std::int32_t idx = 0;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &idx);
    aFrame->code++; // ParamEnd
    if (!aOut || idx < 0)
        return;
    const auto* page = ModMenu::Backend::Get().GetPageByIndex(modId.c_str(), static_cast<std::size_t>(idx));
    *aOut = page ? RED4ext::CString(page->title.c_str()) : RED4ext::CString("");
}

static void ModMenu_GetEntryCount_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, std::int32_t* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    aFrame->code++; // ParamEnd
    if (!aOut)
        return;
    *aOut = static_cast<std::int32_t>(ModMenu::Backend::Get().GetEntryCount(modId.c_str(), pageId.c_str()));
}

static void ModMenu_GetEntryId_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    std::int32_t idx = 0;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    RED4ext::GetParameter(aFrame, &idx);
    aFrame->code++; // ParamEnd
    if (!aOut || idx < 0)
        return;
    const auto* entry =
        ModMenu::Backend::Get().GetEntryByIndex(modId.c_str(), pageId.c_str(), static_cast<std::size_t>(idx));
    *aOut = entry ? RED4ext::CString(entry->id.c_str()) : RED4ext::CString("");
}

static void ModMenu_GetEntryTitle_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                                    std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    std::int32_t idx = 0;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    RED4ext::GetParameter(aFrame, &idx);
    aFrame->code++; // ParamEnd
    if (!aOut || idx < 0)
        return;
    const auto* entry =
        ModMenu::Backend::Get().GetEntryByIndex(modId.c_str(), pageId.c_str(), static_cast<std::size_t>(idx));
    *aOut = entry ? RED4ext::CString(entry->title.c_str()) : RED4ext::CString("");
}

static void ModMenu_GetEntryType_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, std::int32_t* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    std::int32_t idx = 0;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    RED4ext::GetParameter(aFrame, &idx);
    aFrame->code++; // ParamEnd
    if (!aOut || idx < 0)
        return;
    const auto* entry =
        ModMenu::Backend::Get().GetEntryByIndex(modId.c_str(), pageId.c_str(), static_cast<std::size_t>(idx));
    *aOut = entry ? static_cast<std::int32_t>(entry->type) : -1;
}

static void ModMenu_GetToggleValue_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    RED4ext::CString entryId;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    RED4ext::GetParameter(aFrame, &entryId);
    aFrame->code++; // ParamEnd
    if (!aOut)
        return;
    const auto* entry = ModMenu::Backend::Get().GetEntryById(modId.c_str(), pageId.c_str(), entryId.c_str());
    *aOut = (entry && entry->type == MODMENU_ENTRY_TOGGLE) ? entry->boolValue : false;
}

static void ModMenu_SetToggleValue_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    RED4ext::CString entryId;
    bool value = false;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    RED4ext::GetParameter(aFrame, &entryId);
    RED4ext::GetParameter(aFrame, &value);
    aFrame->code++; // ParamEnd
    const bool ok = ModMenu::Backend::Get().SetToggle(modId.c_str(), pageId.c_str(), entryId.c_str(), value);
    if (aOut)
        *aOut = ok;
}

static void ModMenu_GetSliderValue_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, float* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    RED4ext::CString entryId;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    RED4ext::GetParameter(aFrame, &entryId);
    aFrame->code++; // ParamEnd
    if (!aOut)
        return;
    const auto* entry = ModMenu::Backend::Get().GetEntryById(modId.c_str(), pageId.c_str(), entryId.c_str());
    *aOut = (entry && entry->type == MODMENU_ENTRY_SLIDER) ? entry->floatValue : 0.0f;
}

static void ModMenu_SetSliderValue_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    RED4ext::CString entryId;
    float value = 0.0f;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    RED4ext::GetParameter(aFrame, &entryId);
    RED4ext::GetParameter(aFrame, &value);
    aFrame->code++; // ParamEnd
    const bool ok = ModMenu::Backend::Get().SetSlider(modId.c_str(), pageId.c_str(), entryId.c_str(), value);
    if (aOut)
        *aOut = ok;
}

static void ModMenu_PressButton_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aOut, std::int64_t)
{
    RED4ext::CString modId;
    RED4ext::CString pageId;
    RED4ext::CString entryId;
    RED4ext::GetParameter(aFrame, &modId);
    RED4ext::GetParameter(aFrame, &pageId);
    RED4ext::GetParameter(aFrame, &entryId);
    aFrame->code++; // ParamEnd
    const bool ok = ModMenu::Backend::Get().PressButton(modId.c_str(), pageId.c_str(), entryId.c_str());
    if (aOut)
        *aOut = ok;
}

static void ModMenu_ScanIncompatiblePlugins_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                                               std::int64_t)
{
    aFrame->code++; // ParamEnd
    if (!aOut)
        return;
    const auto s = ModMenu::Backend::Get().ScanIncompatiblePlugins();
    *aOut = RED4ext::CString(s.c_str());
}

static void ModMenu_GenerateReportBundle_Fn(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                                           std::int64_t)
{
    std::int32_t maxBytes = 16384;
    RED4ext::GetParameter(aFrame, &maxBytes);
    aFrame->code++; // ParamEnd
    if (!aOut)
        return;
    const auto s = ModMenu::Backend::Get().GenerateReportBundle(static_cast<std::size_t>(std::max(0, maxBytes)));
    *aOut = RED4ext::CString(s.c_str());
}

// ============================================================================
// Sample self-registration (v0.1)
// ============================================================================

static void ModMenu_OnDemoToggle(const ModMenuEntryPath*, bool aValue)
{
    ModMenu::Backend::Get().DemoToggleChanged(aValue);
}

static void RegisterBridgeTypes()
{
    // no custom RTTI types yet
}

static void RegisterBridgeFunctions()
{
    auto rtti = RED4ext::CRTTISystem::Get();
    if (!rtti)
        return;

    RED4ext::CBaseFunction::Flags flags = {.isNative = true, .isStatic = true};

    // State
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_IsOpen", "ModMenu_IsOpen", &ModMenu_IsOpen_Fn);
        func->flags = flags;
        func->SetReturnType("Bool");
        rtti->RegisterFunction(func);
    }

    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_SetOpen", "ModMenu_SetOpen", &ModMenu_SetOpen_Fn);
        func->flags = flags;
        func->SetReturnType("Bool");
        func->AddParam("Bool", "open");
        rtti->RegisterFunction(func);
    }

    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_GetRed4extLogTail", "ModMenu_GetRed4extLogTail",
                                                     &ModMenu_GetRed4extLogTail_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("Int32", "maxBytes");
        rtti->RegisterFunction(func);
    }

    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_GetPluginLogTail", "ModMenu_GetPluginLogTail",
                                                     &ModMenu_GetPluginLogTail_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("String", "modId");
        func->AddParam("Int32", "maxBytes");
        rtti->RegisterFunction(func);
    }

    // Model enumeration
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_GetModCount", "ModMenu_GetModCount", &ModMenu_GetModCount_Fn);
        func->flags = flags;
        func->SetReturnType("Int32");
        rtti->RegisterFunction(func);
    }
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_GetModId", "ModMenu_GetModId", &ModMenu_GetModId_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("Int32", "index");
        rtti->RegisterFunction(func);
    }
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_GetModName", "ModMenu_GetModName", &ModMenu_GetModName_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("Int32", "index");
        rtti->RegisterFunction(func);
    }
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_GetPageCount", "ModMenu_GetPageCount", &ModMenu_GetPageCount_Fn);
        func->flags = flags;
        func->SetReturnType("Int32");
        func->AddParam("String", "modId");
        rtti->RegisterFunction(func);
    }
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_GetPageId", "ModMenu_GetPageId", &ModMenu_GetPageId_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("String", "modId");
        func->AddParam("Int32", "index");
        rtti->RegisterFunction(func);
    }
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_GetPageTitle", "ModMenu_GetPageTitle", &ModMenu_GetPageTitle_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("String", "modId");
        func->AddParam("Int32", "index");
        rtti->RegisterFunction(func);
    }
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_GetEntryCount", "ModMenu_GetEntryCount", &ModMenu_GetEntryCount_Fn);
        func->flags = flags;
        func->SetReturnType("Int32");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        rtti->RegisterFunction(func);
    }
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_GetEntryId", "ModMenu_GetEntryId", &ModMenu_GetEntryId_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        func->AddParam("Int32", "index");
        rtti->RegisterFunction(func);
    }
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_GetEntryTitle", "ModMenu_GetEntryTitle", &ModMenu_GetEntryTitle_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        func->AddParam("Int32", "index");
        rtti->RegisterFunction(func);
    }
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_GetEntryType", "ModMenu_GetEntryType", &ModMenu_GetEntryType_Fn);
        func->flags = flags;
        func->SetReturnType("Int32");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        func->AddParam("Int32", "index");
        rtti->RegisterFunction(func);
    }

    // Value get/set + actions
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_GetToggleValue", "ModMenu_GetToggleValue", &ModMenu_GetToggleValue_Fn);
        func->flags = flags;
        func->SetReturnType("Bool");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        func->AddParam("String", "entryId");
        rtti->RegisterFunction(func);
    }
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_SetToggleValue", "ModMenu_SetToggleValue", &ModMenu_SetToggleValue_Fn);
        func->flags = flags;
        func->SetReturnType("Bool");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        func->AddParam("String", "entryId");
        func->AddParam("Bool", "value");
        rtti->RegisterFunction(func);
    }
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_GetSliderValue", "ModMenu_GetSliderValue", &ModMenu_GetSliderValue_Fn);
        func->flags = flags;
        func->SetReturnType("Float");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        func->AddParam("String", "entryId");
        rtti->RegisterFunction(func);
    }
    {
        auto func =
            RED4ext::CGlobalFunction::Create("ModMenu_SetSliderValue", "ModMenu_SetSliderValue", &ModMenu_SetSliderValue_Fn);
        func->flags = flags;
        func->SetReturnType("Bool");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        func->AddParam("String", "entryId");
        func->AddParam("Float", "value");
        rtti->RegisterFunction(func);
    }
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_PressButton", "ModMenu_PressButton", &ModMenu_PressButton_Fn);
        func->flags = flags;
        func->SetReturnType("Bool");
        func->AddParam("String", "modId");
        func->AddParam("String", "pageId");
        func->AddParam("String", "entryId");
        rtti->RegisterFunction(func);
    }

    // Tooling / diagnostics
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_ScanIncompatiblePlugins", "ModMenu_ScanIncompatiblePlugins",
                                                     &ModMenu_ScanIncompatiblePlugins_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        rtti->RegisterFunction(func);
    }
    {
        auto func = RED4ext::CGlobalFunction::Create("ModMenu_GenerateReportBundle", "ModMenu_GenerateReportBundle",
                                                     &ModMenu_GenerateReportBundle_Fn);
        func->flags = flags;
        func->SetReturnType("String");
        func->AddParam("Int32", "logMaxBytes");
        rtti->RegisterFunction(func);
    }
}

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason,
                                        const RED4ext::Sdk* aSdk)
{
    switch (aReason)
    {
    case RED4ext::EMainReason::Load:
    {
        ModMenu::Backend::Get().SetSdk(aHandle, aSdk);

        // Register ModMenu itself so there is always at least one page visible.
        {
            const auto* api = ModMenu_GetApi();
            if (api && api->RegisterMod && api->RegisterPage && api->RegisterToggle)
            {
                ModMenuModInfo mod{
                    .modId = {.ptr = "ModMenu"},
                    .name = {.ptr = "ModMenu"},
                    .author = {.ptr = "macOS Cyberpunk Modding"},
                    .version = {.ptr = MODMENU_VERSION_STR},
                    .capabilityFlags = MODMENU_CAP_UI,
                };
                api->RegisterMod(&mod);

                ModMenuPageInfo page{.pageId = {.ptr = "main"}, .title = {.ptr = "Main"}};
                api->RegisterPage("ModMenu", &page);

                ModMenuToggleInfo demoToggle{
                    .entryId = {.ptr = "demo_toggle"},
                    .title = {.ptr = "Demo toggle"},
                    .defaultValue = false,
                    .onChanged = &ModMenu_OnDemoToggle,
                };
                api->RegisterToggle("ModMenu", "main", &demoToggle);
            }
        }

        // Best-effort: discover other plugins exporting ModMenu_Register.
        ModMenu::Backend::Get().DiscoverAndRegisterExternal();

        break;
    }
    case RED4ext::EMainReason::Unload:
        break;
    default:
        break;
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
{
    aInfo->name = L"ModMenu";
    aInfo->author = L"macOS Cyberpunk Modding";
    aInfo->version = RED4EXT_SEMVER(0, 1, 0);
    // macOS port: RED4ext currently treats "supported runtimes" as the game's exact FileVer.
    // Use INDEPENDENT to bypass the strict FileVer gate for this UI/plugin.
    aInfo->runtime = RED4EXT_RUNTIME_INDEPENDENT;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}

