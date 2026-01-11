cmake_minimum_required(VERSION 3.10)
project(CowWorld)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Assimp (prebuilt) ---
set(ASSIMP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/assimp/include)
set(ASSIMP_LIBRARIES ${CMAKE_SOURCE_DIR}/libs/assimp/lib/assimp-vc143-mt.lib)

# --- GLFW (prebuilt) ---
set(GLFW_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/glfw/include)
set(GLFW_LIBRARIES ${CMAKE_SOURCE_DIR}/libs/glfw/lib/glfw3.lib)

# --- OpenGL (system on Windows) ---
set(OPENGL_LIBRARIES opengl32.lib)

# Glad (local)
add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)

# ImGui
set(IMGUI_SRC
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    include/imgui
    ${GLFW_INCLUDE_DIRS}
)
target_link_libraries(imgui PUBLIC glad ${GLFW_LIBRARIES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Executable ---
add_executable(CowWorld
  src/animated_mesh.cpp
  src/animated_model.cpp
  src/animation.cpp
  src/animation_loader.cpp
  src/animator.cpp
  src/application.cpp
  src/bone.cpp
  src/camera.cpp
  src/input_manager.cpp
  src/light.cpp
  src/main.cpp
  src/menu_system.cpp
  src/mesh.cpp
  src/model.cpp
  src/scene.cpp
  src/shader.cpp
  src/skybox.cpp
  src/stb_image.cpp
  src/texture.cpp
)

# --- Linking ---
target_link_libraries(CowWorld PRIVATE
    ${ASSIMP_LIBRARIES}
    ${GLFW_LIBRARIES}
    imgui
    glad
    ${OPENGL_LIBRARIES}
)

target_include_directories(CowWorld PRIVATE
    include
    ${ASSIMP_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

# --- Copy DLLs after build ---
add_custom_command(TARGET CowWorld POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/libs/assimp/bin/assimp-vc143-mt.dll
        $<TARGET_FILE_DIR:CowWorld>
)
add_custom_command(TARGET CowWorld POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/libs/glfw/bin/glfw3.dll
        $<TARGET_FILE_DIR:CowWorld>
)

# --- Copy assets/shaders ---
add_custom_command(TARGET CowWorld POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:CowWorld>/shaders
)
if (EXISTS ${CMAKE_SOURCE_DIR}/assets)
    add_custom_command(TARGET CowWorld POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:CowWorld>/assets
    )
endif()
