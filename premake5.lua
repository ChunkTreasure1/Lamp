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

LibraryDir = {}
LibraryDir["PhysX"] = "%{wks.location}/Lamp/vendor/PhysX/lib/%{cfg.buildcfg}"

include "Lamp/vendor/GLFW"
include "Lamp/vendor/imgui"
include "Lamp/vendor/glad"

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
		
		"%{prj.name}/vendor/yaml-cpp/src/**.cpp",
		"%{prj.name}/vendor/yaml-cpp/src/**.h",
		"%{prj.name}/vendor/yaml-cpp/include/**.h",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/include",
		"%{prj.name}/vendor/GLFW/include",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/imgui/",
		"%{prj.name}/vendor/glad/include",
		"%{prj.name}/vendor/glm",
		"%{prj.name}/vendor/stb_image",
		"%{prj.name}/vendor/rapidxml",
		"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/fmod/include",
		"%{prj.name}/vendor/PhysX/include",
		"%{prj.name}/vendor/imnodes/",
		"%{prj.name}/vendor/yaml-cpp/include/",
	}
	
	links 
	{
		"GLFW",
		"ImGui",
		"Glad",		
		"%{LibraryDir.PhysX}/PhysX_static_64.lib",
		"%{LibraryDir.PhysX}/PhysXCharacterKinematic_static_64.lib",
		"%{LibraryDir.PhysX}/PhysXCommon_static_64.lib",
		"%{LibraryDir.PhysX}/PhysXCooking_static_64.lib",	
		"%{LibraryDir.PhysX}/PhysXExtensions_static_64.lib",
		"%{LibraryDir.PhysX}/PhysXFoundation_static_64.lib",
		"%{LibraryDir.PhysX}/PhysXPvdSDK_static_64.lib"	}

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
			"GLFW_INCLUDE_NONE"
		}

		filter "configurations:Debug"
			defines { "LP_DEBUG", "LP_ENABLE_ASSERTS" }
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			runtime "Release"
			optimize "on"
			
			defines
			{
				"NDEBUG",
				"LP_RELEASE"
			}

		filter "configurations:Dist"
			defines { "LP_DIST", "LP_ENABLE_ASSERTS", "NDEBUG" }
			runtime "Release"
			optimize "on"
			
			
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
		"Lamp/src",
		"Lamp/vendor/glm",
		"Lamp/vendor/spdlog/include",
		"Lamp/vendor",
		"Lamp/vendor/glad/include",
		"Lamp/vendor/GLFW/include",
		"Lamp/vendor/imgui/",
		"Lamp/vendor/rapidxml",
		"%{prj.name}/src",
		"Lamp/vendor/assimp/include",
		"Lamp/vendor/fmod/include",
		"Game/src",
		"Lamp/vendor/ImGuizmo/include",
		"Lamp/vendor/bullet/src",
		"Lamp/vendor/imnodes/",
		"Lamp/vendor/yaml-cpp/include/",
	}
	
	libdirs
	{
		gamedir,
		"Lamp/vendor/assimp",
		"Lamp/vendor/fmod"
	}
	
	links
	{
		"Lamp",
		"Game",
		"assimp-vc143-mt.lib",
		"fmodstudio_vc.lib",
		"fmod_vc.lib"
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
			"_CRT_SECURE_NO_WARNINGS"
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
		"Lamp/src",
		"Lamp/vendor/glm",
		"Lamp/vendor/spdlog/include",
		"Lamp/vendor",
		"Lamp/vendor/glad/include",
		"Lamp/vendor/GLFW/include",
		"Lamp/vendor/imgui/",
		"Lamp/vendor/rapidxml",
		"%{prj.name}/src",
		"Lamp/vendor/assimp/include",
		"Lamp/vendor/fmod/include"
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
		"Lamp/src",
		"Lamp/vendor/glm",
		"Lamp/vendor/spdlog/include",
		"Lamp/vendor",
		"Lamp/vendor/glad/include",
		"Lamp/vendor/GLFW/include",
		"Lamp/vendor/imgui/",
		"Lamp/vendor/rapidxml",
		"%{prj.name}/src",
		"Lamp/vendor/assimp/include",
		"Lamp/vendor/fmod/include",
		"Game/src",
		"%{prj.name}/vendor/PhysX/include"
	}

	libdirs
	{
		gamedir,
		"Lamp/vendor/assimp",
		"Lamp/vendor/fmod"
	}

	links
	{
		"Lamp",
		"Game",
		"assimp-vc143-mt.lib",
		"fmodstudio_vc.lib",
		"fmod_vc.lib"
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