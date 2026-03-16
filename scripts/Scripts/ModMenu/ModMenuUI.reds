module ModMenu

// Data classes for UI callbacks (used by InkHooks.reds entry widgets)

public class ModMenuEntryData extends IScriptable {
  public let modId: String;
  public let pageId: String;
  public let entryId: String;
}

public class ModMenuSliderData extends IScriptable {
  public let modId: String;
  public let pageId: String;
  public let entryId: String;
  public let delta: Float;
}
