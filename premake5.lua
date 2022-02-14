workspace "Lamp"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"

include "Lamp/vendor/GLFW"
include "Lamp/vendor/imgui"
include "Lamp/vendor/glad"
include "Lamp/vendor/Optick"

project "Lamp"
	location "Lamp"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	disablewarnings
	{
		"4005"
	}

	linkoptions
	{
		"/ignore:4006",
		"/ignore:4099"
	}

	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "lppch.h"
	pchsource "Lamp/src/lppch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		
		"%{prj.name}/vendor/stb_image/stb/**.h",
		"%{prj.name}/vendor/stb_image/stb/**.cpp",

		"%{prj.name}/vendor/imnodes/**.h",
		"%{prj.name}/vendor/imnodes/**.cpp",

		"%{prj.name}/vendor/VulkanMemoryAllocator/**.h",
		"%{prj.name}/vendor/VulkanMemoryAllocator/**.cpp",
		
		"%{prj.name}/vendor/yaml-cpp/src/**.cpp",
		"%{prj.name}/vendor/yaml-cpp/src/**.h",
		"%{prj.name}/vendor/yaml-cpp/include/**.h"		
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",
		
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.STB}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.ImNodes}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.rapidxml}",
		"%{IncludeDir.ktx2}",
		"%{IncludeDir.Optick}"
	}
	
	libdirs
	{
		"%{LibraryDir.VulkanSDK}",
	}

	links 
	{
		"GLFW",
		"ImGui",
		"Glad",
		"Optick",

		"%{Library.PhysX}",
		"%{Library.PhysXCharacterKinematic}",
		"%{Library.PhysXCommon}",
		"%{Library.PhysXCooking}",
		"%{Library.PhysXExtensions}",
		"%{Library.PhysXFoundation}",
		"%{Library.PhysXPvdSDK}",

		"%{Library.Vulkan}"
	}

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS",
		"PX_PHYSX_STATIC_LIB"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"LP_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE",
			"GLM_FORCE_DEPTH_ZERO_TO_ONE"
		}

		filter "configurations:Debug"
			defines { "LP_DEBUG", "LP_ENABLE_ASSERTS" }
			runtime "Debug"
			symbols "on"

			libdirs
			{
				"%{LibraryDir.VulkanSDK_Debug}"
			}

			links
			{
				"%{Library.ShaderC_Debug}",
				"%{Library.ShaderC_Utils_Debug}",
				"%{Library.SPIRV_Cross_Debug}",
				"%{Library.SPIRV_Cross_GLSL_Debug}",
				"%{Library.SPIRV_Tools_Debug}",
				"%{Library.ktx2_debug}"
			}

		filter "configurations:Release"
			runtime "Release"
			optimize "on"
			
			defines
			{
				"NDEBUG",
				"LP_RELEASE"
			}

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
				"%{Library.ktx2}"
			}

		filter "configurations:Dist"
			defines { "LP_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
				"%{Library.ktx2}"
			}
			
			
project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"
	
	disablewarnings
	{
		"4005"
	}
	
	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")
	gamedir = "bin/" .. outputdir .. "/Game"
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/engine/**.glsl",		

		"Lamp/vendor/ImGuizmo/include/**.h",
		"Lamp/vendor/ImGuizmo/include/**.cpp",
		
		"Lamp/vendor/imnodes/**.h",
		"Lamp/vendor/imnodes/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",

		"Lamp/src",
		"Lamp/vendor",
		"Game/src",

		"%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.fmod}", 
		"%{IncludeDir.ImNodes}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.rapidxml}",
		"%{IncludeDir.ktx2}",
		"%{IncludeDir.Optick}"
	}
	
	libdirs
	{
		gamedir,

		"%{LibraryDir.Assimp}",
		"%{LibraryDir.fmod}",
		"%{LibraryDir.VulkanSDK}",
	}
	
	links
	{
		"Lamp",
		"Game",
		"ImGui",
		"Optick",
		
		"%{Library.Assimp}",
		"%{Library.fmod}",
		"%{Library.fmodstudio}",
		"%{Library.Vulkan}"
	}
	
	linkoptions
	{
		"/WHOLEARCHIVE:Game",
		"/ignore:4099"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"LP_PLATFORM_WINDOWS",
			"_CRT_SECURE_NO_WARNINGS",
			"KHRONOS_STATIC"
		}

		filter "configurations:Debug"
			defines { "LP_DEBUG", "LP_ENABLE_ASSERTS" }
			runtime "Debug"
			symbols "on"

			libdirs
			{
				"%{LibraryDir.VulkanSDK_Debug}"
			}

			links
			{
				"%{Library.ShaderC_Debug}",
				"%{Library.ShaderC_Utils_Debug}",
				"%{Library.SPIRV_Cross_Debug}",
				"%{Library.SPIRV_Cross_GLSL_Debug}",
				"%{Library.SPIRV_Tools_Debug}",
				"%{Library.ktx2_debug}"
			}

		filter "configurations:Release"
			defines "LP_RELEASE"
			runtime "Release"
			optimize "on"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
				"%{Library.ktx2}"
			}

		filter "configurations:Dist"
			defines "LP_DIST"
			runtime "Release"
			optimize "on"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
				"%{Library.ktx2}"
			}
			
project "Game"
	location "Game"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	disablewarnings
	{
		"4005"
	}

	linkoptions 
	{
		"/ignore:4099"
	}

	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",

		"Lamp/src",
		"Lamp/vendor",

		"%{IncludeDir.GLM}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.Optick}"
	}
	
	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"LP_PLATFORM_WINDOWS",
		}

		filter "configurations:Debug"
			defines { "LP_DEBUG", "LP_ENABLE_ASSERTS" }
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines "LP_RELEASE"
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			defines "LP_DIST"
			runtime "Release"
			optimize "on"
			
project "GameLauncher"
	location "GameLauncher"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	disablewarnings
	{
		"4005"
	}

	linkoptions 
	{
		"/ignore:4099"
	}

	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")
	gamedir = "bin/" .. outputdir .. "/Game"
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",

		"Lamp/src",
		"Lamp/vendor",
		"Game/src",

		"%{IncludeDir.GLM}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.Optick}"
	}

	libdirs
	{
		gamedir,
		"%{LibraryDir.Assimp}",
		"%{LibraryDir.fmod}",
		"%{LibraryDir.VulkanSDK}"
	}

	links
	{
		"Lamp",
		"Game",
		"ImGui",

		"%{Library.Assimp}",
		"%{Library.fmod}",
		"%{Library.fmodstudio}",
		"%{Library.Vulkan}"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"LP_PLATFORM_WINDOWS",
		}

		filter "configurations:Debug"
			defines { "LP_DEBUG", "LP_ENABLE_ASSERTS" }
			runtime "Debug"
			symbols "on"

			libdirs
			{
				"%{LibraryDir.VulkanSDK_Debug}"
			}

			links
			{
				"%{Library.ShaderC_Debug}",
				"%{Library.ShaderC_Utils_Debug}",
				"%{Library.SPIRV_Cross_Debug}",
				"%{Library.SPIRV_Cross_GLSL_Debug}",
				"%{Library.SPIRV_Tools_Debug}",
			}

		filter "configurations:Release"
			defines "LP_RELEASE"
			runtime "Release"
			optimize "on"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
			}

		filter "configurations:Dist"
			defines "LP_DIST"
			runtime "Release"
			optimize "on"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
			}