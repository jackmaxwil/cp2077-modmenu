#include "../CatchSetup.hpp"
#include "../mocks/MockInk.hpp"
#include "../mocks/MockRed4.hpp"
#include <vector>

TEST_CASE("Ink Widget Creation", "[ink][widget]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Create basic widget") {
        auto* widget = Mock::inkWidget::Create(Mock::inkWidgetType::Widget);
        REQUIRE(widget != nullptr);
        REQUIRE(widget->type == Mock::inkWidgetType::Widget);
    }
    
    SECTION("Create compound widget") {
        auto* compound = Mock::inkCompoundWidget::Create();
        REQUIRE(compound != nullptr);
        REQUIRE(compound->type == Mock::inkWidgetType::Compound);
        REQUIRE(compound->children.empty());
    }
    
    SECTION("Create text widget") {
        auto* text = Mock::inkTextWidget::Create();
        REQUIRE(text != nullptr);
        REQUIRE(text->type == Mock::inkWidgetType::Text);
        REQUIRE(text->text.empty());
        REQUIRE(text->color == 0xFF00FF00);
    }
}

TEST_CASE("Ink Library Operations", "[ink][library]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Get library") {
        auto* lib = Mock::inkLibrary::Get();
        REQUIRE(lib != nullptr);
    }
    
    SECTION("Spawn widget with parent") {
        auto* parent = Mock::inkCompoundWidget::Create();
        auto* lib = Mock::inkLibrary::Get();
        
        auto* child = lib->SpawnWidget(parent, "TestWidget");
        REQUIRE(child != nullptr);
        REQUIRE(child->parent == parent);
        REQUIRE(parent->children.size() == 1);
        REQUIRE(parent->children[0] == child);
    }
    
    SECTION("Spawn text widget") {
        auto* parent = Mock::inkCompoundWidget::Create();
        auto* lib = Mock::inkLibrary::Get();
        
        auto* text = lib->SpawnText(parent, "Hello World");
        REQUIRE(text != nullptr);
        REQUIRE(text->parent == parent);
        REQUIRE(std::string(text->text) == "Hello World");
        REQUIRE(parent->children.size() == 1);
    }
    
    SECTION("Multiple child widgets") {
        auto* parent = Mock::inkCompoundWidget::Create();
        auto* lib = Mock::inkLibrary::Get();
        
        lib->SpawnWidget(parent, "Child1");
        lib->SpawnWidget(parent, "Child2");
        lib->SpawnWidget(parent, "Child3");
        
        REQUIRE(parent->children.size() == 3);
    }
}

TEST_CASE("Ink Widget Removal", "[ink][widget]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Remove widget from parent") {
        auto* parent = Mock::inkCompoundWidget::Create();
        auto* lib = Mock::inkLibrary::Get();
        
        auto* child = lib->SpawnWidget(parent, "TestWidget");
        REQUIRE(parent->children.size() == 1);
        
        lib->RemoveWidget(child);
        REQUIRE(parent->children.size() == 0);
    }
    
    SECTION("Remove specific child from multiple") {
        auto* parent = Mock::inkCompoundWidget::Create();
        auto* lib = Mock::inkLibrary::Get();
        
        auto* child1 = lib->SpawnWidget(parent, "Child1");
        auto* child2 = lib->SpawnWidget(parent, "Child2");
        auto* child3 = lib->SpawnWidget(parent, "Child3");
        
        lib->RemoveWidget(child2);
        
        REQUIRE(parent->children.size() == 2);
        REQUIRE(parent->children[0] == child1);
        REQUIRE(parent->children[1] == child3);
    }
}

TEST_CASE("Text Widget Properties", "[ink][text]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Default text is empty") {
        auto* text = Mock::inkTextWidget::Create();
        REQUIRE(text->text.empty());
    }
    
    SECTION("Set text through spawn") {
        auto* parent = Mock::inkCompoundWidget::Create();
        auto* lib = Mock::inkLibrary::Get();
        
        auto* text = lib->SpawnText(parent, "Custom Text");
        REQUIRE(std::string(text->text) == "Custom Text");
    }
    
    SECTION("Default color is green") {
        auto* text = Mock::inkTextWidget::Create();
        REQUIRE(text->color == 0xFF00FF00);
    }
}

TEST_CASE("Widget Visibility", "[ink][widget]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Default visibility is true") {
        auto* widget = Mock::inkWidget::Create(Mock::inkWidgetType::Widget);
        REQUIRE(widget->visible == true);
    }
    
    SECTION("Toggle visibility") {
        auto* widget = Mock::inkWidget::Create(Mock::inkWidgetType::Widget);
        
        widget->visible = false;
        REQUIRE(widget->visible == false);
        
        widget->visible = true;
        REQUIRE(widget->visible == true);
    }
}

TEST_CASE("Widget Handle Management", "[ink][widget]") {
    SETUP_TEST_ENVIRONMENT();
    
    SECTION("Widget has valid handle") {
        auto* widget = Mock::inkWidget::Create(Mock::inkWidgetType::Widget);
        REQUIRE(static_cast<bool>(widget->handle) == true);
    }
    
    SECTION("Handle points to widget") {
        auto* compound = Mock::inkCompoundWidget::Create();
        auto widgetPtr = reinterpret_cast<uint64_t>(compound);
        auto handleValue = static_cast<uint64_t>(compound->handle);
        
        REQUIRE(handleValue == widgetPtr);
    }
}