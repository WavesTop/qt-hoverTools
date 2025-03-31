cmake_minimum_required(VERSION 3.16)

# 项目名称和版本
project(hoverTools VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 Qt 的 moc/uic/rcc
set(CMAKE_AUTOMOC ON)    # 自动生成 moc 文件
set(CMAKE_AUTOUIC ON)    # 自动处理 UI 文件（.ui）
set(CMAKE_AUTORCC ON)    # 自动处理资源文件（.qrc）

# 查找 Qt 库（优先查找 Qt6，若未找到则回退到 Qt5）
if(NOT DEFINED Qt6_DIR)
  message(STATUS "Searching for Qt6...")
  find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)
else()
  find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)
endif()

# 如果未找到 Qt6，尝试查找 Qt5
if(NOT Qt6_FOUND)
  message(STATUS "Qt6 not found, trying Qt5...")
  find_package(Qt5 5.15 COMPONENTS Core Widgets Gui REQUIRED)
endif()

# 添加可执行文件
add_executable(${PROJECT_NAME}
  main.cpp
  mainwindow.cpp
  mainwindow.h
)

# 链接 Qt 库
if(Qt6_FOUND)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
  )
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
  )
endif()