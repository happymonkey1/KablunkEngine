#include "kablunkpch.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"

#define CR_HOST // tell cr.h that this is the host application
#include <cr.h>

namespace Kablunk
{
	static cr_plugin s_ctx;

	void NativeScriptEngine::Init()
	{
		s_ctx = {};
	}

	void NativeScriptEngine::Open(const char* path)
	{
		cr_plugin_open(s_ctx, path);
	}

	bool NativeScriptEngine::Update()
	{
		if (!s_ctx.p)
			return true;

		return cr_plugin_update(s_ctx);
	}

	void NativeScriptEngine::Shutdown()
	{
		cr_plugin_close(s_ctx);
	}

	Scope<NativeScriptInterface> NativeScriptEngine::GetScript(const std::string& name)
	{
		return Scope<NativeScriptInterface>(GetScriptFromRegistry(name));
	}
}


#if 0

#include "Kablunk/Scripts/NativeScriptEngine.h"

#include "Kablunk/Utilities/Parser.h"
#include "Kablunk/Utilities/Utilities.h"

#include "Kablunk/Scripts/NativeScriptCompilerLogger.h"
#include "Kablunk/Scripts/NativeScriptModule.h"


namespace Kablunk
{
	// Static functions
	void NativeScriptEngine::Init()
	{
		if (!s_native_script_engine)
			s_native_script_engine = new NativeScriptEngine{};
	}

	void NativeScriptEngine::Shutdown()
	{
		//delete s_native_script_engine;
	}

	bool NativeScriptEngine::AreScriptsCompiling()
	{
		{
			if (s_native_script_engine)
				return Get()->m_runtime_object_system->GetIsCompiling();
			else
				return false;
		}
	}

	NativeScriptEngine::NativeScriptEngine()
		: m_compiler_logger{ new NativeScriptCompilerLogger{} }, m_runtime_object_system{ new RuntimeObjectSystem{} }
	{
		if (!m_runtime_object_system->Initialise(m_compiler_logger, nullptr))
		{
			std::cout << "something failed with runtime compiled c plus plus" << std::endl;
			exit(1);
		}

		m_runtime_object_system->GetObjectFactorySystem()->AddListener(this);
		m_runtime_object_system->AddIncludeDir("C:\\Users\\Gaming Account\\Source\\Repos\\KablunkEngine2020\\Sandbox\\src\\Sandbox");
		m_runtime_object_system->AddIncludeDir("C:\\Users\\Gaming Account\\Source\\Repos\\KablunkEngine2020\\KablunkEngine\\include");
		m_runtime_object_system->AddIncludeDir("C:\\Users\\Gaming Account\\Source\\Repos\\KablunkEngine2020\\KablunkEngine\\vendor");

		m_runtime_object_system->SetAdditionalLinkOptions("/LIBPATH:C:\\Users\\Gaming Account\\Source\\Repos\\KablunkEngine2020\\bin\\ KablunkEngine.lib");
		
	}

	NativeScriptEngine::~NativeScriptEngine()
	{
		if (m_runtime_object_system)
			m_runtime_object_system->CleanObjectFiles();

		if (m_runtime_object_system && m_runtime_object_system->GetObjectFactorySystem())
		{
			m_runtime_object_system->GetObjectFactorySystem()->RemoveListener(this);

			for (auto& [script_name, nsc_runtime] : m_native_scripts)
			{
				if (nsc_runtime.ptr)
				{
					IObject* obj_ptr = m_runtime_object_system->GetObjectFactorySystem()->GetObjectW(nsc_runtime.id);
					delete obj_ptr;
				}
			}
		}

		delete m_runtime_object_system;
		delete m_compiler_logger;
	}

	bool NativeScriptEngine::RegisterScript(const std::string& script_name, CreateMethodFunc create_script)
	{
		KB_CORE_ASSERT(false, "deprecated");
		/*
		auto& native_scripts = GetScriptContainer();
		auto it = native_scripts.find(script_name);
		if (it == native_scripts.end())
		{
			native_scripts.emplace( script_name, create_script );
			return true;
		}
		else
		{
			KB_CORE_ASSERT(false, "Script already registered!")
			return false;
		}*/
		return false;
	}

	// #TODO currently searches for files during runtime, probably better to do at statically with reflection
	Scope<NativeScriptInterface> NativeScriptEngine::GetScript(const std::string& script_name)
	{
		KB_CORE_ASSERT(false, "deprecated");
		//auto it = m_native_scripts.find(script_name);
		//return it != m_native_scripts.end() ? Scope<NativeScript>(it->second()) : nullptr;
		return nullptr;
	}

	bool NativeScriptEngine::LoadDLLRuntime(const std::string& dll_name, const std::string& dll_dir)
	{
		NativeScriptModule nsc_module = NativeScriptModule{ dll_name, dll_dir };

		if (nsc_module)
		{
			//auto dll_func = nsc_module["DebugPrint"];

			/*if (dll_func != NULL)
			{
				successful_link = true;
				dll_func();
			}*/
		}
		else
		{
			KB_CORE_ERROR("Failed to load dll '{0}'", dll_name);
			return false;
		}

		return true;
	}

	NativeScriptInterface** NativeScriptEngine::AddScript(const std::string& name, const std::filesystem::path& filepath)
	{
		auto filename = filepath.filename();
		m_runtime_object_system->AddToRuntimeFileList(filename.string().c_str());
		
		// #TODO support multiple native scripts 
		auto it = m_native_scripts.find(name);
		if (it == m_native_scripts.end())
		{
			IObjectConstructor* ctr_ptr = m_runtime_object_system->GetObjectFactorySystem()->GetConstructor(name.c_str());
			
			if (ctr_ptr)
			{
				IObject* obj_ptr = ctr_ptr->Construct();
				NativeScriptInterface* nsc_ptr = nullptr;
				obj_ptr->GetInterface(&nsc_ptr);
				if (!nsc_ptr)
				{
					delete obj_ptr;
					KB_CORE_ERROR("could not find native script interface");
					return nullptr;
				}

				Internal::NativeScriptRuntimeObject nsc_runtime = { nsc_ptr, obj_ptr->GetObjectId() };
				m_native_scripts[name] = nsc_runtime;
				return &(m_native_scripts[name].ptr);
			}
		}

		return nullptr;
	}

	void NativeScriptEngine::OnConstructorsAdded()
	{
		for (auto& [script_name, nsc_runtime] : m_native_scripts)
		{
			if (nsc_runtime.ptr)
			{
				IObject* obj_ptr = m_runtime_object_system->GetObjectFactorySystem()->GetObjectW(nsc_runtime.id);
				obj_ptr->GetInterface(&nsc_runtime.ptr);
				if (!obj_ptr)
				{
					delete obj_ptr;
					nsc_runtime.ptr = nullptr;
					KB_CORE_ERROR("could not find native script interface");
					break;
				}
			}
		}
	}

	void NativeScriptEngine::OnUpdate(Timestep ts)
	{
		// #TODO add to job
		if (m_runtime_object_system->GetIsCompiledComplete())
		{
			KB_CORE_INFO("loading compiled module");
			m_runtime_object_system->LoadCompiledModule();
		}

		if (!m_runtime_object_system->GetIsCompiling())
		{
			m_runtime_object_system->GetFileChangeNotifier()->Update(ts);
		}
	}
}

#endif
