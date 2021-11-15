//#include "RCCPP/RuntimeObjectSystem/ObjectInterfacePerModule.h"
//#include "RCCPP/RuntimeObjectSystem/IObject.h"
//#include <kablunk.h>
#include "TestScript.h"

#include "Sandbox/ReflectionTest.h"
#include <kablunk.h>

//#include "RCCPP/RuntimeObjectSystem/RuntimeInclude.h"
//RUNTIME_MODIFIABLE_INCLUDE;

class TestScript : public TInterface<Kablunk::InterfaceIDs::NATIVE_SCRIPT, Kablunk::NativeScript>
{
public:
	float Deltatime = 0;

	TestScript() { }
	~TestScript() = default;

	void OnAwake() override
	{
		KB_CLIENT_TRACE("TestScript::OnAwake()");

		/*auto reflect = TestStruct::Reflection;
		for (const auto& member : reflect.Members)
		{
			KB_CLIENT_TRACE("REFLECTION: '({0}) {1}'", member.GetTypeName(), member.GetName());
		}
		*/

	}

	void OnUpdate(Kablunk::Timestep ts) override
	{
		Deltatime += ts;
		if (Deltatime >= 1.0f)
		{
			Deltatime -= 1.0f;
			KB_CLIENT_TRACE("TestScript::OnUpdate()");
		}
	}

	void OnDestroy() override
	{
		KB_CLIENT_TRACE("TestScript::OnDestroy()");
	}
};

REGISTERCLASS(TestScript);
