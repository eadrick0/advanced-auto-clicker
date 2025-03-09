cmake_minimum_required(VERSION 3.10)
project(AdvancedAutoClicker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Ana uygulama
add_executable(auto_clicker main.cpp)

target_link_libraries(auto_clicker PRIVATE
    imgui
    d3d11
    dxgi
    d3dcompiler
)

# Windows spesifik ayarlar
if(WIN32)
    target_link_libraries(auto_clicker PRIVATE
        user32
        gdi32
        shell32
    )
endif()