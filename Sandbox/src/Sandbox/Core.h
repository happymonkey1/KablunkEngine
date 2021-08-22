#ifndef SANDBOX_CORE_H
#define SANDBOX_CORE_H

#include "Sandbox/TestScript.h"
#include "Sandbox/CameraController.h"
#include "Sandbox/ReflectionTest.h"
// #TODO Remove
#include "Eclipse/EclipseSandbox.h"


REGISTER_NATIVE_SCRIPT(CameraController);
REGISTER_NATIVE_SCRIPT(TestScript);

// #TODO Remove
namespace Eclipse
{
	REGISTER_NATIVE_SCRIPT(ReadTokensTest);
}
#endif
