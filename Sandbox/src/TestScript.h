#ifndef SANDBOX_TEST_SCRIPT_H
#define SANDBOX_TEST_SCRIPT_H

#include <kablunk.h>

struct TestScript : public Kablunk::NativeScript
{

	TestScript(Kablunk::Entity entity)
		: Kablunk::NativeScript(entity)
	{
		
	}


	virtual void OnAwake() override
	{
		KB_CLIENT_TRACE("TestScript::OnAwake()");
	}

	virtual void OnUpdate(Kablunk::Timestep ts) override
	{
		KB_CLIENT_TRACE("TestScript::OnUpdate()");
	}
};

#endif
