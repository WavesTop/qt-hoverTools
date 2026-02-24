cmake_minimum_required(VERSION 3.16)
project(hoverTools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE AND EXISTS "/opt/homebrew/opt/qt")
  list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")
endif()
if(APPLE AND EXISTS "/usr/local/opt/qt")
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt")
endif()

set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
  find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)
endif()

set(SOURCES
  main.cpp
  plugins/IActionProvider.cpp
  core/UserShortcutsProvider.cpp
  ui/FloatingAssistant.cpp
  ui/ToolbarPanel.cpp
)
add_executable(hoverTools ${SOURCES})

target_include_directories(hoverTools PRIVATE
  ${CMAKE_SOURCE_DIR}/ui
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_SOURCE_DIR}/plugins
)

if(TARGET Qt6::Widgets)
  target_link_libraries(hoverTools PRIVATE Qt6::Widgets)
else()
  target_link_libraries(hoverTools PRIVATE Qt5::Widgets)
endif()

if(APPLE)
  set(LINK_FILE "${CMAKE_BINARY_DIR}/CMakeFiles/hoverTools.dir/link.txt")
  add_custom_command(TARGET hoverTools PRE_LINK
    COMMAND sed -i.bak "s/-framework AGL//g" "CMakeFiles/hoverTools.dir/link.txt"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "移除已废弃的 AGL 框架引用"
  )
  set_target_properties(hoverTools PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.tophovertools.hoverTools"
  )
endif()
