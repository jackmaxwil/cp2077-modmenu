#include "MockInk.hpp"

namespace Mock {

static std::vector<std::unique_ptr<inkWidget>> g_inkWidgetStorage;

inkWidget* inkWidget::Create(inkWidgetType type) {
    auto widget = std::make_unique<inkWidget>();
    widget->type = type;
    widget->handle = Handle::FromPtr(reinterpret_cast<uint64_t>(widget.get()));
    
    inkWidget* ptr = widget.get();
    g_inkWidgetStorage.push_back(std::move(widget));
    return ptr;
}

void inkWidget::Destroy(inkWidget* widget) {
    if (widget && widget->parent) {
        auto it = std::find(widget->parent->children.begin(), 
                          widget->parent->children.end(), 
                          widget);
        if (it != widget->parent->children.end()) {
            widget->parent->children.erase(it);
        }
    }
}

inkCompoundWidget* inkCompoundWidget::Create() {
    auto widget = std::make_unique<inkCompoundWidget>();
    widget->type = inkWidgetType::Compound;
    widget->handle = Handle::FromPtr(reinterpret_cast<uint64_t>(widget.get()));
    
    inkCompoundWidget* ptr = widget.get();
    g_inkWidgetStorage.push_back(std::move(widget));
    return ptr;
}

inkTextWidget* inkTextWidget::Create() {
    auto widget = std::make_unique<inkTextWidget>();
    widget->type = inkWidgetType::Text;
    widget->handle = Handle::FromPtr(reinterpret_cast<uint64_t>(widget.get()));
    
    inkTextWidget* ptr = widget.get();
    g_inkWidgetStorage.push_back(std::move(widget));
    return ptr;
}

inkLibrary* inkLibrary::Get() {
    static inkLibrary library;
    return &library;
}

inkCompoundWidget* inkLibrary::SpawnWidget(inkCompoundWidget* parent, const char* name) {
    auto widget = inkCompoundWidget::Create();
    widget->parent = parent;
    if (parent) {
        parent->children.push_back(widget);
    }
    return widget;
}

inkTextWidget* inkLibrary::SpawnText(inkCompoundWidget* parent, const char* text) {
    auto widget = inkTextWidget::Create();
    widget->text = text ? text : "";
    widget->parent = parent;
    if (parent) {
        parent->children.push_back(widget);
    }
    return widget;
}

void inkLibrary::RemoveWidget(inkWidget* widget) {
    inkWidget::Destroy(widget);
}

void inkWidgetLibrary::Initialize() {
    s_spawnLocalCallback = nullptr;
    s_removeWidgetCallback = nullptr;
}

void inkWidgetLibrary::Shutdown() {
    g_inkWidgetStorage.clear();
}

std::function<Handle(inkCompoundWidget*, const ResourceAsyncReference&)> 
    inkWidgetLibrary::s_spawnLocalCallback = nullptr;

std::function<void(inkWidget*)> inkWidgetLibrary::s_removeWidgetCallback = nullptr;

void ClearMockInk() {
    g_inkWidgetStorage.clear();
}

} // namespace Mock