#include "kablunkpch.h"

#include "Kablunk/Scripts/NativeScript.h"

namespace Kablunk
{
	NativeScript::NativeScript()
		: m_entity{}
	{

	}

	NativeScript::NativeScript(Entity entity)
		: m_entity{ entity }
	{
	
	}
}

