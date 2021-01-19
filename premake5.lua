workspace "Lamp"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Lamp/vendor/GLFW"
include "Lamp/vendor/imgui"
include "Lamp/vendor/glad"
include "Lamp/vendor/bullet"

project "Lamp"
	location "Lamp"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"%{prj.name}/vendor/stb_image/stb/**.cpp"
	}

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS"
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
		"%{prj.name}/vendor/bullet/src"
	}
	
	links 
	{
		"GLFW",
		"ImGui",
		"Glad",
		"Bullet"
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
			defines "LP_RELEASE"
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			defines "LP_DIST"
			runtime "Release"
			optimize "on"
			
project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")
	gamedir = "bin/" .. outputdir .. "/Game"
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/engine/**.vert",
		"%{prj.name}/engine/**.frag",
		"Lamp/vendor/ImGuizmo/include/**.h",
		"Lamp/vendor/ImGuizmo/include/**.cpp"
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
		"Lamp/vendor/bullet/src"
	}

	libdirs
	{
		gamedir,
		"Lamp/vendor/assimp",
		"Lamp/vendor/fmod",
		"Lamp/vendor/bullet"
	}

	links
	{
		"Game",
		"Lamp",
		"assimp-vc142-mt.lib",
		"fmodstudio_vc.lib",
		"fmod_vc.lib"
	}
	
	linkoptions
	{
		"/WHOLEARCHIVE:Game",
		"/WHOLEARCHIVE:Lamp"
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
			
project "Game"
	location "Game"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"Lamp/vendor/fmod/include",
		"Lamp/vendor/bullet/src"
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
	staticruntime "on"

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
		"Lamp/vendor/bullet/src"
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
		"assimp-vc142-mt.lib",
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