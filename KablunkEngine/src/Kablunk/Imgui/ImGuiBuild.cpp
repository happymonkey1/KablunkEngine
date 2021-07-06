#include "kablunkpch.h"

#include "Kablunk/Imgui/ImGuiBuild.h"


#define IMGUI_IMPL_OPENGL_LOADER_GLAD

// #IMPORTANT when updating to latest imgui branch, g_Data was redefined. Changed one of the "g_Data" -> "g_OpenGL_Data"
#include <backends/imgui_impl_glfw.cpp>
#include <backends/imgui_impl_opengl3.cpp>

