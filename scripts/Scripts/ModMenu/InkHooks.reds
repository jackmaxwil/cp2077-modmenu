module ModMenu

import ModMenu.Events.*

// =============================================================================
// Input listener (PlayerPuppet) - toggles mod menu action
// =============================================================================

public class ModMenuListener {
  private let player: wref<PlayerPuppet>;

  public func SetPlayer(player: ref<PlayerPuppet>) -> Void {
    this.player = player;
  }

  protected cb func OnAction(action: ListenerAction, consumer: ListenerActionConsumer) -> Bool {
    if !IsDefined(this.player) {
      return false;
    };

    if ListenerAction.IsAction(action, n"modmenu_toggle") && Equals(ListenerAction.GetType(action), gameinputActionType.BUTTON_RELEASED) {
      this.player.modmenuActive = !this.player.modmenuActive;
      let evt: ref<ModMenuToggleEvent> = new ModMenuToggleEvent();
      evt.open = this.player.modmenuActive;
      GameInstance.GetUISystem(this.player.GetGame()).QueueEvent(evt);
      return true;
    };

    return false;
  }
}

@addField(PlayerPuppet)
public let modmenuActive: Bool;

@addField(PlayerPuppet)
private let modmenuListener: ref<ModMenuListener>;

@wrapMethod(PlayerPuppet)
protected cb func OnGameAttached() -> Bool {
  wrappedMethod();

  // Register input listener once.
  if !IsDefined(this.modmenuListener) {
    this.modmenuListener = new ModMenuListener();
    this.modmenuListener.SetPlayer(this);
    this.RegisterInputListener(this.modmenuListener);
  };

  // Show by default (first hotkey press toggles OFF).
  this.modmenuActive = true;
  let evt: ref<ModMenuToggleEvent> = new ModMenuToggleEvent();
  evt.open = true;
  GameInstance.GetUISystem(this.GetGame()).QueueEvent(evt);
}

@wrapMethod(PlayerPuppet)
protected cb func OnDetach() -> Bool {
  wrappedMethod();
  if IsDefined(this.modmenuListener) {
    this.UnregisterInputListener(this.modmenuListener);
    this.modmenuListener = null;
  };
}

// =============================================================================
// UI (ink) - apply toggle event on a HUD controller
// =============================================================================

@addField(inkGameController)
private let modmenuWidget: wref<inkWidget>;

@addMethod(inkGameController)
protected cb func OnModMenuToggleEvent(evt: ref<ModMenuToggleEvent>) -> Bool {
  if !this.IsA(n"gameuiRootHudGameController") {
    return false;
  };

  this.EnsureModMenuWidget();
  if IsDefined(this.modmenuWidget) {
    this.modmenuWidget.SetVisible(evt.open);
  };
}

@addMethod(inkGameController)
private func EnsureModMenuWidget() -> Void {
  if IsDefined(this.modmenuWidget) {
    return;
  };

  let root: wref<inkCompoundWidget> = this.GetRootCompoundWidget() as inkCompoundWidget;
  if !IsDefined(root) {
    return;
  };

  // v0.1: use an existing inkwidget as a guaranteed-visible placeholder overlay.
  this.modmenuWidget = this.SpawnFromExternal(
    root,
    r"base\\gameplay\\gui\\widgets\\streaming_spinner\\streaming_spinner.inkwidget",
    n"Root"
  );
}

