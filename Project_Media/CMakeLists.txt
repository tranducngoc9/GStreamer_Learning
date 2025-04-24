cmake_minimum_required(VERSION 3.5)
 
project(Gstreamer_GUI LANGUAGES CXX)
 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
 
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DGST_DEBUG_BIN_TO_DOT_FILE_ENABLED)
 
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Quick Widgets QuickControls2 REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
 
add_definitions(-DGST)
 
if(ANDROID)
    add_library(Gstreamer_GUI SHARED main.cpp qml.qrc)
else()
    add_executable(Gstreamer_GUI main.cpp qml.qrc customdata.h customdata.cpp)
endif()
 
# Thêm các đường dẫn tới thư viện và include
include_directories(${GST_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
message("GST_INCLUDE_DIRS: " ${GST_INCLUDE_DIRS})
 
target_compile_definitions(Gstreamer_GUI
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(Gstreamer_GUI
  PUBLIC ${GST_LIBRARIES}
  PRIVATE Qt5::Core Qt5::Quick Qt5::QuickControls2)