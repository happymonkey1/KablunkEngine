#ifndef SANDBOX_TEST_SCRIPT_H
#define SANDBOX_TEST_SCRIPT_H

#include "Sandbox/ReflectionTest.h"
#include <kablunk.h>

struct TestScript : public Kablunk::NativeScript
{
	TestScript()
	{
		
	}
	virtual ~TestScript() = default;

	IMPLEMENT_NATIVE_SCRIPT(TestScript);

	virtual void OnAwake() override
	{
		KB_CLIENT_TRACE("TestScript::OnAwake()");
	}

	virtual void OnUpdate(Kablunk::Timestep ts) override
	{
		KB_CLIENT_TRACE("TestScript::OnUpdate()");

		auto reflect = TestStruct::Reflection;
		for (const auto& member : reflect.Members)
		{
			KB_CLIENT_TRACE("REFLECTION: '({0}) {1}'", member.GetTypeName(), member.GetName());
		}
	}

	virtual void OnDestroy() override
	{
		KB_CLIENT_TRACE("TestScript::OnDestroy()");
	}
};


#endif
