cmake_minimum_required(VERSION 3.20)

project(ModMenu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(MODMENU_BUILD_INSTALL_SCRIPTS "Include scripts/ in install" ON)

# RED4ext.SDK headers. RED4EXT_SDK_DIR is the SDK repository root; it defaults to the pinned vendor/RED4ext.SDK
# submodule. RED4ext's tools/cp-dev and scripts/create_release.sh pass the workspace checkout explicitly.
set(RED4EXT_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/vendor/RED4ext.SDK" CACHE PATH "RED4ext.SDK repository root")
if (NOT EXISTS "${RED4EXT_SDK_DIR}/include/RED4ext/RED4ext.hpp")
  message(FATAL_ERROR "RED4ext.SDK not found at ${RED4EXT_SDK_DIR}. "
                      "Run `git submodule update --init --recursive`, or pass -DRED4EXT_SDK_DIR=<path>.")
endif()
message(STATUS "RED4ext.SDK: ${RED4EXT_SDK_DIR}")

add_library(ModMenu SHARED
  src/main.cpp
  src/modmenu_backend.cpp
  src/modmenu_backend.hpp
)

target_include_directories(ModMenu
  PRIVATE
    "${RED4EXT_SDK_DIR}/include"
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
