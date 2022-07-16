#pragma once

//FOR USE BY CLIENT APPLICATIONS

// ---Core---------------------------------------
#include "Kablunk/Core/Logger.h"
#include "Kablunk/Core/Application.h"
#include "Kablunk/Core/Layer.h"
#include "Kablunk/Imgui/ImGuiLayer.h"
#include "Kablunk/Core/Input.h"

#include "Kablunk/Core/Timing.h"
#include "Kablunk/Core/Timestep.h"

#include "Kablunk/Core/JobSystem.h"
#include "Kablunk/Core/UUID64.h"
#include "Kablunk/Core/RefCounting.h"
// ----------------------------------------------


// ---Renderer-----------------------------------
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Renderer2D.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Framebuffer.h"
#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/SceneRenderer.h"

#include "Kablunk/Renderer/Camera.h"
// ----------------------------------------------


// ---ECS----------------------------------------
#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Scene/Scene.h"
#include "Kablunk/Scene/Components.h"
#include "Kablunk/Scene/SceneCamera.h"
// ----------------------------------------------

// ---Scripts------------------------------------
#include "Kablunk/Scripts/NativeScript.h"
#include "Kablunk/Scripts/NativeScriptEngine.h"
// ----------------------------------------------

// ----------------------------------------------

// ---Assets-------------------------------------
#include "Kablunk/Core/AssetManager.h"
// ----------------------------------------------

// ---Reflection---------------------------------
#include "Kablunk/Utilities/Reflection/ReflectionCore.h"
// ----------------------------------------------
 
// ---Utilities----------------------------------
#include "Kablunk/Utilities/BinarySearch.hpp"
#include "Kablunk/Utilities/Containers/PriorityQueue.hpp"
// ----------------------------------------------

// ---Misc---------------------------------------
#include "Platform/PlatformAPI.h"
#include "imgui.h"
//#include "imgui_internal.h"
// ----------------------------------------------



