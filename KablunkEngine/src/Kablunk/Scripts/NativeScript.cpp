#include "kablunkpch.h"

#include "Kablunk/Scripts/NativeScript.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"

namespace Kablunk
{

	void NativeScript::Destroy()
	{
		WeakRef<Scene> scene = NativeScriptEngine::Get()->GetScene();

		KB_CORE_ASSERT(scene, "scene not set!");

		scene->DestroyEntity(m_entity);
	}

}

