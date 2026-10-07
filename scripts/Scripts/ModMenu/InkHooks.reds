module ModMenu


// =============================================================================
// ModMenu overlay — hooks the HUD controller to inject the full settings UI.
// Toggle via F10 (modmenu_toggle action).
// =============================================================================

@addField(inkGameController)
private let modmenuStatusLabel: wref<inkText>;

@addField(inkGameController)
private let modmenuRootCanvas: wref<inkCanvas>;

@addField(inkGameController)
private let modmenuMainPanel: wref<inkCanvas>;

@addField(inkGameController)
private let modmenuSidebar: wref<inkCanvas>;

@addField(inkGameController)
private let modmenuModListContent: wref<inkVerticalPanel>;

@addField(inkGameController)
private let modmenuSettingsContent: wref<inkVerticalPanel>;

@addField(inkGameController)
private let modmenuPageSelector: wref<inkHorizontalPanel>;

@addField(inkGameController)
private let modmenuTitleText: wref<inkText>;

@addField(inkGameController)
private let modmenuModButtons: array<wref<inkWidget>>;

@addField(inkGameController)
private let modmenuCurrentModId: String;

@addField(inkGameController)
private let modmenuCurrentPageId: String;

@addField(inkGameController)
private let modmenuVisible: Bool;

// The overlay lives on the popups manager: its root covers the whole screen and it exists for the whole time the
// player is in the world. (The health bar's root, used before, is small and fades out at full health.)
@wrapMethod(PopupsManager)
protected cb func OnInitialize() -> Bool {
  let result = wrappedMethod();
  let root = this.GetRootCompoundWidget();
  if IsDefined(root) {
    let size = root.GetSize();
    ModMenu_Log("hud: PopupsManager.OnInitialize, root " + NameToString(root.GetName()) + " size " + ToString(size.X) + "x"
      + ToString(size.Y) + " visible " + ToString(root.IsVisible()) + " opacity " + ToString(root.GetOpacity()));
  } else {
    ModMenu_Log("hud: PopupsManager.OnInitialize, no root widget");
  }
  this.ModMenu_CreateStatusLabel();
  this.ModMenu_CreateFullUI();
  // OnPlayerAttach may have run before this controller initialized.
  let player = this.GetPlayerControlledObject() as PlayerPuppet;
  if IsDefined(player) {
    player.modmenuOverlay = this;
    ModMenu_Log("hud: overlay attached to the player (from OnInitialize)");
  }
  return result;
}

@wrapMethod(PopupsManager)
protected cb func OnPlayerAttach(playerPuppet: ref<GameObject>) -> Bool {
  let result = wrappedMethod(playerPuppet);
  let player = playerPuppet as PlayerPuppet;
  if IsDefined(player) {
    player.modmenuOverlay = this;
    ModMenu_Log("hud: overlay attached to the player");
  }
  return result;
}

// =============================================================================
// Status label (always visible when ModMenu is loaded)
// =============================================================================

@addMethod(inkGameController)
private func ModMenu_CreateStatusLabel() -> Void {
  let root = this.GetRootCompoundWidget();
  if !IsDefined(root) {
    return;
  }

  let label = new inkText();
  label.SetName(n"ModMenuStatusLabel");
  label.SetText("ModMenu Active - press ` (or F10)");
  label.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  label.SetFontStyle(n"Medium");
  label.SetFontSize(18);
  label.SetTintColor(Color(0, 200, 80, 200));
  label.SetMargin(inkMargin(50.0, 50.0, 0.0, 0.0));
  label.SetAnchor(inkEAnchor.TopLeft);
  label.SetOpacity(0.6);
  root.AddChildWidget(label);
  this.modmenuStatusLabel = label;
  ModMenu_Log("hud: status label added");
}

// =============================================================================
// Full UI overlay (hidden until ` or F10)
// =============================================================================

@addMethod(inkGameController)
private func ModMenu_CreateFullUI() -> Void {
  let root = this.GetRootCompoundWidget();
  if !IsDefined(root) {
    return;
  }

  // Full-screen overlay. Layout below uses canvases, where a child's margin is its offset from the top-left;
  // stacking panels (horizontal/vertical) would lay the backgrounds and the content out side by side instead.
  let canvas = new inkCanvas();
  canvas.SetName(n"ModMenuOverlay");
  canvas.SetAnchor(inkEAnchor.Fill);
  canvas.SetVisible(false);
  root.AddChildWidget(canvas);
  this.modmenuRootCanvas = canvas;

  let bg = new inkRectangle();
  bg.SetName(n"ModMenuBg");
  bg.SetAnchor(inkEAnchor.Fill);
  bg.SetTintColor(Color(0, 0, 0, 180));
  bg.SetOpacity(0.8);
  canvas.AddChildWidget(bg);

  // Main panel, 1200x700, centred on screen.
  let mainPanel = new inkCanvas();
  mainPanel.SetName(n"ModMenuMainPanel");
  mainPanel.SetSize(1200.0, 700.0);
  mainPanel.SetAnchor(inkEAnchor.Centered);
  mainPanel.SetAnchorPoint(0.5, 0.5);
  // Laid out in 1200x700 units, shown at 65% so it leaves the scene visible around it.
  mainPanel.SetRenderTransformPivot(0.5, 0.5);
  mainPanel.SetScale(Vector2(0.65, 0.65));
  canvas.AddChildWidget(mainPanel);
  this.modmenuMainPanel = mainPanel;

  let panelBg = new inkRectangle();
  panelBg.SetAnchor(inkEAnchor.Fill);
  panelBg.SetTintColor(Color(20, 20, 20, 255));
  panelBg.SetOpacity(0.95);
  mainPanel.AddChildWidget(panelBg);

  this.ModMenu_CreateHeader();
  this.ModMenu_CreateSidebar();
  this.ModMenu_CreateSettingsPanel();

  this.modmenuVisible = false;
}

@addMethod(inkGameController)
private func ModMenu_CreateHeader() -> Void {
  let title = new inkText();
  title.SetName(n"ModMenuTitle");
  title.SetText("ModMenu");
  title.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  title.SetFontStyle(n"Medium");
  title.SetFontSize(32);
  title.SetTintColor(Color(255, 255, 255, 255));
  title.SetMargin(inkMargin(20.0, 15.0, 0.0, 0.0));
  this.modmenuMainPanel.AddChildWidget(title);
  this.modmenuTitleText = title;
}

@addMethod(inkGameController)
private func ModMenu_CreateSidebar() -> Void {
  // Left column: 300x620 at (10, 70) inside the main panel.
  let sidebar = new inkCanvas();
  sidebar.SetName(n"ModMenuSidebar");
  sidebar.SetSize(300.0, 620.0);
  sidebar.SetMargin(inkMargin(10.0, 70.0, 0.0, 0.0));
  this.modmenuMainPanel.AddChildWidget(sidebar);

  let sidebarBg = new inkRectangle();
  sidebarBg.SetAnchor(inkEAnchor.Fill);
  sidebarBg.SetTintColor(Color(30, 30, 30, 255));
  sidebarBg.SetOpacity(0.9);
  sidebar.AddChildWidget(sidebarBg);

  let label = new inkText();
  label.SetText("Installed Mods");
  label.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  label.SetFontStyle(n"Medium");
  label.SetFontSize(20);
  label.SetTintColor(Color(200, 200, 200, 255));
  label.SetMargin(inkMargin(15.0, 15.0, 0.0, 0.0));
  sidebar.AddChildWidget(label);

  // Mod buttons stack vertically below the label.
  let content = new inkVerticalPanel();
  content.SetName(n"ModMenuModListContent");
  content.SetMargin(inkMargin(10.0, 50.0, 0.0, 0.0));
  sidebar.AddChildWidget(content);
  this.modmenuModListContent = content;
  this.modmenuSidebar = sidebar;
}

@addMethod(inkGameController)
private func ModMenu_CreateSettingsPanel() -> Void {
  // Right column: 860x620 at (320, 70) inside the main panel.
  let panel = new inkCanvas();
  panel.SetName(n"ModMenuSettingsPanel");
  panel.SetSize(860.0, 620.0);
  panel.SetMargin(inkMargin(320.0, 70.0, 0.0, 0.0));
  this.modmenuMainPanel.AddChildWidget(panel);

  let settingsBg = new inkRectangle();
  settingsBg.SetAnchor(inkEAnchor.Fill);
  settingsBg.SetTintColor(Color(40, 40, 40, 255));
  settingsBg.SetOpacity(0.9);
  panel.AddChildWidget(settingsBg);

  // Page tabs along the top.
  let tabs = new inkHorizontalPanel();
  tabs.SetName(n"ModMenuPageTabs");
  tabs.SetMargin(inkMargin(10.0, 10.0, 0.0, 0.0));
  panel.AddChildWidget(tabs);
  this.modmenuPageSelector = tabs;

  // Settings entries stack vertically below the tabs.
  let content = new inkVerticalPanel();
  content.SetName(n"ModMenuSettingsContent");
  content.SetMargin(inkMargin(10.0, 60.0, 0.0, 0.0));
  panel.AddChildWidget(content);
  this.modmenuSettingsContent = content;

  let placeholder = new inkText();
  placeholder.SetText("Select a mod from the list to view its settings");
  placeholder.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  placeholder.SetFontStyle(n"Medium");
  placeholder.SetFontSize(18);
  placeholder.SetTintColor(Color(150, 150, 150, 255));
  placeholder.SetMargin(inkMargin(10.0, 10.0, 0.0, 0.0));
  content.AddChildWidget(placeholder);
}

// =============================================================================
// Toggle event — show/hide the full UI, refresh mod list on open
// =============================================================================

// Called directly by the input listener on the one controller that owns the overlay (no broadcast event: an
// event handler added to inkGameController ran in every UI controller in the game on each key press).
@addMethod(inkGameController)
public func ModMenu_Toggle(open: Bool) -> Void {
  this.modmenuVisible = open;
  ModMenu_SetOpen(open);
  ModMenu_Log("hud: toggle " + ToString(open) + ", overlay defined: " + ToString(IsDefined(this.modmenuRootCanvas)));

  if IsDefined(this.modmenuRootCanvas) {
    this.modmenuRootCanvas.SetVisible(open);
  }

  if open {
    this.ModMenu_RefreshModList();
    if IsDefined(this.modmenuStatusLabel) {
      this.modmenuStatusLabel.SetVisible(false);
    }
  } else {
    if IsDefined(this.modmenuStatusLabel) {
      this.modmenuStatusLabel.SetVisible(true);
    }
  }
}

// =============================================================================
// Mod list population
// =============================================================================

@addMethod(inkGameController)
private func ModMenu_RefreshModList() -> Void {
  if !IsDefined(this.modmenuModListContent) {
    return;
  }

  this.modmenuModListContent.RemoveAllChildren();
  ArrayClear(this.modmenuModButtons);

  let modCount = ModMenu_GetModCount();

  if modCount == 0 {
    let noMods = new inkText();
    noMods.SetText("No mods registered");
    noMods.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
    noMods.SetFontStyle(n"Medium");
    noMods.SetFontSize(16);
    noMods.SetTintColor(Color(120, 120, 120, 255));
    noMods.SetMargin(inkMargin(15.0, 10.0, 0.0, 0.0));
    this.modmenuModListContent.AddChildWidget(noMods);
    return;
  }

  let idx = 0;
  while idx < modCount {
    let modId = ModMenu_GetModId(idx);
    let modName = ModMenu_GetModName(idx);
    if StrLen(modName) == 0 {
      modName = modId;
    }

    let btn = new inkText();
    btn.SetName(StringToName("ModBtn_" + IntToString(idx)));
    btn.SetText(modName);
    btn.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
    btn.SetFontStyle(n"Medium");
    btn.SetFontSize(16);
    btn.SetTintColor(Color(180, 180, 180, 255));
    btn.SetSize(240.0, 35.0);
    btn.SetMargin(inkMargin(10.0, 5.0, 10.0, 5.0));
    btn.RegisterToCallback(n"OnRelease", this, n"OnModMenu_ModSelected");

    this.modmenuModListContent.AddChildWidget(btn);
    ArrayPush(this.modmenuModButtons, btn);

    idx += 1;
  }
}

@addMethod(inkGameController)
protected cb func OnModMenu_ModSelected(widget: wref<inkWidget>, userData: ref<IScriptable>) -> Bool {
  let idx = this.ModMenu_FindModButtonIndex(widget);
  if idx >= 0 {
    let modId = ModMenu_GetModId(idx);
    this.modmenuCurrentModId = modId;
    this.ModMenu_LoadModSettings(modId);

    // Highlight selected
    let i = 0;
    while i < ArraySize(this.modmenuModButtons) {
      let btn = this.modmenuModButtons[i] as inkText;
      if IsDefined(btn) {
        if i == idx {
          btn.SetTintColor(Color(0, 180, 255, 255));
        } else {
          btn.SetTintColor(Color(180, 180, 180, 255));
        }
      }
      i += 1;
    }
  }
  return true;
}

@addMethod(inkGameController)
private func ModMenu_FindModButtonIndex(widget: wref<inkWidget>) -> Int32 {
  let i = 0;
  while i < ArraySize(this.modmenuModButtons) {
    if this.modmenuModButtons[i] == widget {
      return i;
    }
    i += 1;
  }
  return -1;
}

// =============================================================================
// Settings panel population
// =============================================================================

@addMethod(inkGameController)
private func ModMenu_LoadModSettings(modId: String) -> Void {
  if !IsDefined(this.modmenuSettingsContent) || !IsDefined(this.modmenuPageSelector) {
    return;
  }

  this.modmenuSettingsContent.RemoveAllChildren();
  this.modmenuPageSelector.RemoveAllChildren();

  // Update title
  let modName = "";
  let modCount = ModMenu_GetModCount();
  let mi = 0;
  while mi < modCount {
    if Equals(ModMenu_GetModId(mi), modId) {
      modName = ModMenu_GetModName(mi);
    }
    mi += 1;
  }
  if StrLen(modName) == 0 {
    modName = modId;
  }
  if IsDefined(this.modmenuTitleText) {
    this.modmenuTitleText.SetText("ModMenu - " + modName);
  }

  let pageCount = ModMenu_GetPageCount(modId);
  if pageCount == 0 {
    let noSettings = new inkText();
    noSettings.SetText("This mod has no configurable settings");
    noSettings.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
    noSettings.SetFontStyle(n"Medium");
    noSettings.SetFontSize(16);
    noSettings.SetTintColor(Color(150, 150, 150, 255));
    noSettings.SetMargin(inkMargin(20.0, 20.0, 0.0, 0.0));
    this.modmenuSettingsContent.AddChildWidget(noSettings);
    return;
  }

  // Create page tabs
  let pi = 0;
  while pi < pageCount {
    let pageId = ModMenu_GetPageId(modId, pi);
    let pageTitle = ModMenu_GetPageTitle(modId, pi);
    if StrLen(pageTitle) == 0 {
      pageTitle = pageId;
    }

    let tab = new inkText();
    tab.SetName(StringToName("PageTab_" + IntToString(pi)));
    tab.SetText(pageTitle);
    tab.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
    tab.SetFontStyle(n"Medium");
    tab.SetFontSize(14);
    tab.SetTintColor(Color(180, 180, 180, 255));
    tab.SetSize(120.0, 30.0);
    tab.SetMargin(inkMargin(5.0, 5.0, 5.0, 5.0));
    tab.RegisterToCallback(n"OnRelease", this, n"OnModMenu_PageSelected");
    this.modmenuPageSelector.AddChildWidget(tab);

    pi += 1;
  }

  // Load first page
  if pageCount > 0 {
    let firstPageId = ModMenu_GetPageId(modId, 0);
    this.ModMenu_LoadPageEntries(modId, firstPageId);
  }
}

@addMethod(inkGameController)
protected cb func OnModMenu_PageSelected(widget: wref<inkWidget>, userData: ref<IScriptable>) -> Bool {
  let name = widget.GetName();
  let nameStr = NameToString(name);

  // Extract page index from "PageTab_N"
  let pageIdx = 0;
  let pageCount = ModMenu_GetPageCount(this.modmenuCurrentModId);
  while pageIdx < pageCount {
    if Equals(nameStr, "PageTab_" + IntToString(pageIdx)) {
      let pageId = ModMenu_GetPageId(this.modmenuCurrentModId, pageIdx);
      this.ModMenu_LoadPageEntries(this.modmenuCurrentModId, pageId);
      break;
    }
    pageIdx += 1;
  }
  return true;
}

@addMethod(inkGameController)
private func ModMenu_LoadPageEntries(modId: String, pageId: String) -> Void {
  if !IsDefined(this.modmenuSettingsContent) {
    return;
  }

  this.modmenuCurrentPageId = pageId;
  this.modmenuSettingsContent.RemoveAllChildren();

  let entryCount = ModMenu_GetEntryCount(modId, pageId);
  let ei = 0;
  while ei < entryCount {
    let entryId = ModMenu_GetEntryId(modId, pageId, ei);
    let entryTitle = ModMenu_GetEntryTitle(modId, pageId, ei);
    let entryType = ModMenu_GetEntryType(modId, pageId, ei);
    if StrLen(entryTitle) == 0 {
      entryTitle = entryId;
    }

    switch entryType {
      case 0:
        this.ModMenu_CreateToggle(modId, pageId, entryId, entryTitle);
        break;
      case 1:
        this.ModMenu_CreateSlider(modId, pageId, entryId, entryTitle);
        break;
      case 2:
        this.ModMenu_CreateActionButton(modId, pageId, entryId, entryTitle);
        break;
      default:
        this.ModMenu_CreateLabel(entryTitle);
    }
    ei += 1;
  }
}

// =============================================================================
// Entry widgets
// =============================================================================

@addMethod(inkGameController)
private func ModMenu_CreateToggle(modId: String, pageId: String, entryId: String, title: String) -> Void {
  let row = new inkHorizontalPanel();
  row.SetSize(800.0, 40.0);
  row.SetMargin(inkMargin(10.0, 10.0, 10.0, 5.0));

  let label = new inkText();
  label.SetText(title);
  label.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  label.SetFontStyle(n"Medium");
  label.SetFontSize(16);
  label.SetTintColor(Color(220, 220, 220, 255));
  label.SetSize(600.0, 30.0);
  row.AddChildWidget(label);

  let value = ModMenu_GetToggleValue(modId, pageId, entryId);

  let toggle = new inkText();
  toggle.SetName(StringToName("toggle_" + entryId));
  toggle.SetText(value ? "ON" : "OFF");
  toggle.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  toggle.SetFontStyle(n"Medium");
  toggle.SetFontSize(16);
  toggle.SetSize(80.0, 30.0);
  toggle.SetTintColor(value ? Color(0, 200, 80, 255) : Color(200, 60, 60, 255));
  toggle.RegisterToCallback(n"OnRelease", this, n"OnModMenu_TogglePressed");
  row.AddChildWidget(toggle);

  this.modmenuSettingsContent.AddChildWidget(row);
}

@addMethod(inkGameController)
protected cb func OnModMenu_TogglePressed(widget: wref<inkWidget>, userData: ref<IScriptable>) -> Bool {
  let name = NameToString(widget.GetName());
  // Name is "toggle_<entryId>"
  let entryId = StrAfterFirst(name, "toggle_");
  if StrLen(entryId) > 0 && StrLen(this.modmenuCurrentModId) > 0 && StrLen(this.modmenuCurrentPageId) > 0 {
    let cur = ModMenu_GetToggleValue(this.modmenuCurrentModId, this.modmenuCurrentPageId, entryId);
    let next = !cur;
    ModMenu_SetToggleValue(this.modmenuCurrentModId, this.modmenuCurrentPageId, entryId, next);

    let text = widget as inkText;
    if IsDefined(text) {
      text.SetText(next ? "ON" : "OFF");
      text.SetTintColor(next ? Color(0, 200, 80, 255) : Color(200, 60, 60, 255));
    }
  }
  return true;
}

@addMethod(inkGameController)
private func ModMenu_CreateSlider(modId: String, pageId: String, entryId: String, title: String) -> Void {
  let container = new inkVerticalPanel();
  container.SetSize(800.0, 55.0);
  container.SetMargin(inkMargin(10.0, 10.0, 10.0, 5.0));

  let labelRow = new inkHorizontalPanel();
  labelRow.SetSize(800.0, 25.0);

  let label = new inkText();
  label.SetText(title);
  label.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  label.SetFontStyle(n"Medium");
  label.SetFontSize(16);
  label.SetTintColor(Color(220, 220, 220, 255));
  labelRow.AddChildWidget(label);

  let value = ModMenu_GetSliderValue(modId, pageId, entryId);
  let valText = new inkText();
  valText.SetName(StringToName("sliderval_" + entryId));
  valText.SetText(FloatToString(value));
  valText.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  valText.SetFontStyle(n"Medium");
  valText.SetFontSize(14);
  valText.SetTintColor(Color(180, 180, 180, 255));
  valText.SetMargin(inkMargin(20.0, 0.0, 0.0, 0.0));
  labelRow.AddChildWidget(valText);
  container.AddChildWidget(labelRow);

  let ctrlRow = new inkHorizontalPanel();
  ctrlRow.SetSize(800.0, 30.0);
  ctrlRow.SetMargin(inkMargin(0.0, 5.0, 0.0, 0.0));

  let decBtn = new inkText();
  decBtn.SetName(StringToName("sliderdec_" + entryId));
  decBtn.SetText(" - ");
  decBtn.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  decBtn.SetFontStyle(n"Medium");
  decBtn.SetFontSize(18);
  decBtn.SetTintColor(Color(200, 200, 200, 255));
  decBtn.SetSize(40.0, 30.0);
  decBtn.RegisterToCallback(n"OnRelease", this, n"OnModMenu_SliderDecPressed");
  ctrlRow.AddChildWidget(decBtn);

  let bar = new inkRectangle();
  bar.SetName(StringToName("sliderbar_" + entryId));
  bar.SetSize(300.0, 20.0);
  bar.SetMargin(inkMargin(10.0, 5.0, 10.0, 0.0));
  bar.SetTintColor(Color(0, 100, 200, 255));
  ctrlRow.AddChildWidget(bar);

  let incBtn = new inkText();
  incBtn.SetName(StringToName("sliderinc_" + entryId));
  incBtn.SetText(" + ");
  incBtn.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  incBtn.SetFontStyle(n"Medium");
  incBtn.SetFontSize(18);
  incBtn.SetTintColor(Color(200, 200, 200, 255));
  incBtn.SetSize(40.0, 30.0);
  incBtn.RegisterToCallback(n"OnRelease", this, n"OnModMenu_SliderIncPressed");
  ctrlRow.AddChildWidget(incBtn);

  container.AddChildWidget(ctrlRow);
  this.modmenuSettingsContent.AddChildWidget(container);
}

@addMethod(inkGameController)
protected cb func OnModMenu_SliderDecPressed(widget: wref<inkWidget>, userData: ref<IScriptable>) -> Bool {
  let name = NameToString(widget.GetName());
  let entryId = StrAfterFirst(name, "sliderdec_");
  this.ModMenu_AdjustSlider(entryId, -0.1);
  return true;
}

@addMethod(inkGameController)
protected cb func OnModMenu_SliderIncPressed(widget: wref<inkWidget>, userData: ref<IScriptable>) -> Bool {
  let name = NameToString(widget.GetName());
  let entryId = StrAfterFirst(name, "sliderinc_");
  this.ModMenu_AdjustSlider(entryId, 0.1);
  return true;
}

@addMethod(inkGameController)
private func ModMenu_AdjustSlider(entryId: String, delta: Float) -> Void {
  if StrLen(entryId) == 0 || StrLen(this.modmenuCurrentModId) == 0 || StrLen(this.modmenuCurrentPageId) == 0 {
    return;
  }
  let cur = ModMenu_GetSliderValue(this.modmenuCurrentModId, this.modmenuCurrentPageId, entryId);
  let next = cur + delta;
  if next < 0.0 { next = 0.0; }
  if next > 10.0 { next = 10.0; }
  ModMenu_SetSliderValue(this.modmenuCurrentModId, this.modmenuCurrentPageId, entryId, next);

  // Refresh entire page to update display
  this.ModMenu_LoadPageEntries(this.modmenuCurrentModId, this.modmenuCurrentPageId);
}

@addMethod(inkGameController)
private func ModMenu_CreateActionButton(modId: String, pageId: String, entryId: String, title: String) -> Void {
  let row = new inkHorizontalPanel();
  row.SetSize(800.0, 40.0);
  row.SetMargin(inkMargin(10.0, 10.0, 10.0, 5.0));

  let label = new inkText();
  label.SetText(title);
  label.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  label.SetFontStyle(n"Medium");
  label.SetFontSize(16);
  label.SetTintColor(Color(220, 220, 220, 255));
  label.SetSize(600.0, 30.0);
  row.AddChildWidget(label);

  let actionBtn = new inkText();
  actionBtn.SetName(StringToName("action_" + entryId));
  actionBtn.SetText("[Execute]");
  actionBtn.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  actionBtn.SetFontStyle(n"Medium");
  actionBtn.SetFontSize(14);
  actionBtn.SetTintColor(Color(0, 160, 255, 255));
  actionBtn.SetSize(120.0, 30.0);
  actionBtn.RegisterToCallback(n"OnRelease", this, n"OnModMenu_ActionPressed");
  row.AddChildWidget(actionBtn);

  this.modmenuSettingsContent.AddChildWidget(row);
}

@addMethod(inkGameController)
protected cb func OnModMenu_ActionPressed(widget: wref<inkWidget>, userData: ref<IScriptable>) -> Bool {
  let name = NameToString(widget.GetName());
  let entryId = StrAfterFirst(name, "action_");
  if StrLen(entryId) > 0 && StrLen(this.modmenuCurrentModId) > 0 && StrLen(this.modmenuCurrentPageId) > 0 {
    ModMenu_PressButton(this.modmenuCurrentModId, this.modmenuCurrentPageId, entryId);
  }
  return true;
}

@addMethod(inkGameController)
private func ModMenu_CreateLabel(title: String) -> Void {
  let label = new inkText();
  label.SetText(title);
  label.SetFontFamily("base\\gameplay\\gui\\fonts\\raj\\raj.inkfontfamily");
  label.SetFontStyle(n"Medium");
  label.SetFontSize(14);
  label.SetTintColor(Color(180, 180, 180, 255));
  label.SetMargin(inkMargin(15.0, 10.0, 0.0, 5.0));
  this.modmenuSettingsContent.AddChildWidget(label);
}

// =============================================================================
// Input listener — F10 toggle
// =============================================================================

@addField(PlayerPuppet)
private let modmenuListener: ref<ModMenuInputListener>;

@addField(PlayerPuppet)
public let modmenuOpen: Bool;

@addField(PlayerPuppet)
public let modmenuOverlay: wref<inkGameController>;

@wrapMethod(PlayerPuppet)
protected cb func OnGameAttached() -> Bool {
  wrappedMethod();
  
  if !IsDefined(this.modmenuListener) {
    this.modmenuListener = new ModMenuInputListener();
    this.modmenuListener.SetPlayer(this);
    this.RegisterInputListener(this.modmenuListener);
    ModMenu_Log("input: listener registered on PlayerPuppet");
  }
}

@wrapMethod(PlayerPuppet)
protected cb func OnDetach() -> Bool {
  wrappedMethod();
  
  if IsDefined(this.modmenuListener) {
    this.UnregisterInputListener(this.modmenuListener);
    this.modmenuListener = null;
  }
}

public class ModMenuInputListener {
  private let player: wref<PlayerPuppet>;
  private let loggedActions: Int32;

  public func SetPlayer(player: ref<PlayerPuppet>) -> Void {
    this.player = player;
  }

  protected cb func OnAction(action: ListenerAction, consumer: ListenerActionConsumer) -> Bool {
    if !IsDefined(this.player) {
      return false;
    }

    let actionName = ListenerAction.GetName(action);
    let actionType = ListenerAction.GetType(action);
    if this.loggedActions < 200 && Equals(actionType, gameinputActionType.BUTTON_RELEASED) {
      this.loggedActions += 1;
      ModMenu_Log("input: " + NameToString(actionName) + " released");
    }

    if Equals(actionName, n"modmenu_toggle") && Equals(actionType, gameinputActionType.BUTTON_RELEASED) {
      this.player.modmenuOpen = !this.player.modmenuOpen;
      if IsDefined(this.player.modmenuOverlay) {
        this.player.modmenuOverlay.ModMenu_Toggle(this.player.modmenuOpen);
      } else {
        ModMenu_Log("input: toggle pressed but no overlay is attached");
      }
      
      return true;
    }

    return false;
  }
}
