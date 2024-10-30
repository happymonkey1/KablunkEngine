#include "kablunkpch.h"

#include "Kablunk/Scripts/NativeScript.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"

namespace kb
{

	void NativeScript::Destroy()
	{
		weak_arc<Scene> scene = NativeScriptEngine::get().get_scene();

		KB_CORE_ASSERT(scene, "scene not set!");

		scene->DestroyEntity(m_entity);
	}

}

