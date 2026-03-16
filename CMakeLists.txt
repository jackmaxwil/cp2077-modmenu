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

option(MODMENU_BUILD_TESTS "Build unit tests" ON)

if (MODMENU_BUILD_INSTALL_SCRIPTS)
  install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/scripts/Scripts"
    DESTINATION "red4ext/plugins/ModMenu"
  )
  install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/scripts/r6"
    DESTINATION "red4ext/plugins/ModMenu"
  )
  install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/scripts/config"
    DESTINATION "red4ext/plugins/ModMenu"
  )
endif()

# Unit Tests with Catch2
find_package(Catch2 3 QUIET)
if (NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
  )
  FetchContent_MakeAvailable(Catch2)
endif()

if (MODMENU_BUILD_TESTS)
  enable_testing()

  add_executable(ModMenuTests
    tests/CatchSetup.cpp
    tests/mocks/MockRED4ext.cpp
    tests/mocks/MockRed4.cpp
    tests/mocks/MockInk.cpp
    tests/mocks/MockInput.cpp
    tests/mocks/MockAddressResolver.cpp
    tests/unit/test_core.cpp
    tests/unit/test_ink.cpp
    tests/unit/test_input.cpp
    tests/unit/test_addresses.cpp
  )

  target_include_directories(ModMenuTests
    PRIVATE
      "${CMAKE_CURRENT_LIST_DIR}/tests/mocks"
      "${CMAKE_CURRENT_LIST_DIR}/tests"
  )

  target_link_libraries(ModMenuTests
    PRIVATE
      Catch2::Catch2WithMain
  )

  add_test(NAME ModMenuTests COMMAND ModMenuTests)
endif()

