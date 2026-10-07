// Generated from src/main.cpp RegisterBridgeFunctions(); keep in sync. Global (no module): the natives are
// registered under their bare names, and a module-scoped declaration would not bind.

native func ModMenu_Log(message: String) -> Void;
native func ModMenu_IsOpen() -> Bool;
native func ModMenu_SetOpen(open: Bool) -> Bool;
native func ModMenu_GetRed4extLogTail(maxBytes: Int32) -> String;
native func ModMenu_GetPluginLogTail(modId: String, maxBytes: Int32) -> String;
native func ModMenu_GetModCount() -> Int32;
native func ModMenu_GetModId(index: Int32) -> String;
native func ModMenu_GetModName(index: Int32) -> String;
native func ModMenu_GetPageCount(modId: String) -> Int32;
native func ModMenu_GetPageId(modId: String, index: Int32) -> String;
native func ModMenu_GetPageTitle(modId: String, index: Int32) -> String;
native func ModMenu_GetEntryCount(modId: String, pageId: String) -> Int32;
native func ModMenu_GetEntryId(modId: String, pageId: String, index: Int32) -> String;
native func ModMenu_GetEntryTitle(modId: String, pageId: String, index: Int32) -> String;
native func ModMenu_GetEntryType(modId: String, pageId: String, index: Int32) -> Int32;
native func ModMenu_GetToggleValue(modId: String, pageId: String, entryId: String) -> Bool;
native func ModMenu_SetToggleValue(modId: String, pageId: String, entryId: String, value: Bool) -> Bool;
native func ModMenu_GetSliderValue(modId: String, pageId: String, entryId: String) -> Float;
native func ModMenu_SetSliderValue(modId: String, pageId: String, entryId: String, value: Float) -> Bool;
native func ModMenu_PressButton(modId: String, pageId: String, entryId: String) -> Bool;
native func ModMenu_ScanIncompatiblePlugins() -> String;
native func ModMenu_GenerateReportBundle(logMaxBytes: Int32) -> String;
