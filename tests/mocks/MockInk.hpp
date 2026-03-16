#pragma once

#include "MockRED4ext.hpp"
#include "MockRed4.hpp"
#include <cstdint>
#include <functional>
#include <memory>

namespace Mock {

struct inkWidget;
struct inkCompoundWidget;
struct inkTextWidget;
struct inkLibrary;

enum class inkWidgetType {
    Compound,
    Text,
    Button,
    Canvas,
    Widget
};

struct inkWidget {
    static inkWidget* Create(inkWidgetType type);
    static void Destroy(inkWidget* widget);
    
    inkWidgetType type = inkWidgetType::Widget;
    Handle handle;
    inkCompoundWidget* parent = nullptr;
    uint64_t userdata = 0;
    bool visible = true;
};

struct inkCompoundWidget : public inkWidget {
    static inkCompoundWidget* Create();
    
    std::vector<inkWidget*> children;
};

struct inkTextWidget : public inkWidget {
    static inkTextWidget* Create();
    
    std::string text;
    uint32_t color = 0xFF00FF00;
};

struct inkLibrary {
    static inkLibrary* Get();
    
    inkCompoundWidget* SpawnWidget(inkCompoundWidget* parent, const char* name);
    inkTextWidget* SpawnText(inkCompoundWidget* parent, const char* text);
    void RemoveWidget(inkWidget* widget);
};

struct inkWidgetLibrary {
    static void Initialize();
    static void Shutdown();
    
    static std::function<Handle(inkCompoundWidget*, const ResourceAsyncReference&)> s_spawnLocalCallback;
    static std::function<void(inkWidget*)> s_removeWidgetCallback;
};

void ClearMockInk();

} // namespace Mock