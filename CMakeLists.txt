cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(SculptorV2 VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES} 
    "src/hooks/LevelEditorLayer.cpp" 
    "src/lib/NumberInput.cpp" 
    "src/lib/Utilities.cpp" 
    "src/lib/Geometry.cpp" 
    "src/lib/PropertyEditor.cpp" 
    "src/lib/Manager.cpp" 
    "src/hooks/EditorUI.cpp" 
    "src/lib/Interface.cpp" 
    "src/sculptor/Layer.cpp" 
    "src/sculptor/Form.cpp"
    "src/external/clipper.engine.cpp"
    "src/external/clipper.triangulation.cpp" "src/sculptor/BezierEditor.cpp" "src/lib/DragNode.cpp" "src/lib/Modulator.cpp" "src/sculptor/Serialization.cpp" "src/hooks/EditorPauseLayer.cpp")

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
