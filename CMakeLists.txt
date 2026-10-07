cmake_minimum_required(VERSION 3.20)

project(ModMenu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(MODMENU_BUILD_INSTALL_SCRIPTS "Include scripts/ in install" ON)

# Prefer a sibling checkout of RED4ext.SDK, fallback to vendor/RED4ext.SDK if present.
set(MODMENU_RED4EXT_SDK_DIR_DEFAULT "${CMAKE_CURRENT_LIST_DIR}/../RED4ext.SDK")
if (EXISTS "${MODMENU_RED4EXT_SDK_DIR_DEFAULT}/include/RED4ext/RED4ext.hpp")
  set(MODMENU_RED4EXT_SDK_DIR "${MODMENU_RED4EXT_SDK_DIR_DEFAULT}" CACHE PATH "Path to RED4ext.SDK repo root")
else()
  set(MODMENU_RED4EXT_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/vendor/RED4ext.SDK" CACHE PATH "Path to RED4ext.SDK repo root")
endif()

if (NOT EXISTS "${MODMENU_RED4EXT_SDK_DIR}/include/RED4ext/RED4ext.hpp")
  message(FATAL_ERROR "RED4ext.SDK not found. Set -DMODMENU_RED4EXT_SDK_DIR=/path/to/RED4ext.SDK")
endif()

add_library(ModMenu SHARED
  src/main.cpp
  src/modmenu_backend.cpp
  src/modmenu_backend.hpp
)

target_include_directories(ModMenu
  PRIVATE
    "${MODMENU_RED4EXT_SDK_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

target_compile_definitions(ModMenu PRIVATE
  MODMENU_VERSION_STR="0.1.0"
)

if (APPLE)
  target_compile_definitions(ModMenu PRIVATE MODMENU_PLATFORM_APPLE=1)
  target_link_libraries(ModMenu PRIVATE "-Wl,-undefined,dynamic_lookup")
endif()

set_target_properties(ModMenu PROPERTIES
  OUTPUT_NAME "ModMenu"
)

# Convenience install layout matching RED4ext expectations.
install(TARGETS ModMenu
  LIBRARY DESTINATION "red4ext/plugins/ModMenu"
)

if (MODMENU_BUILD_INSTALL_SCRIPTS)
  install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/scripts/Scripts"
    DESTINATION "red4ext/plugins/ModMenu"
  )
  # Input bindings go to the game's r6/input, where inputloader.pl reads them.
  install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/scripts/r6"
    DESTINATION "."
  )
endif()
