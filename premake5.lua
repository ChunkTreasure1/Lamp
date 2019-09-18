workspace "Lamp"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Lamp/vendor/GLFW/include"
IncludeDir["ImGui"] = "Lamp/vendor/imgui/"

include "Lamp/vendor/GLFW"
include "Lamp/vendor/imgui"

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
		"%{prj.name}/src/**.cpp"
	}

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{prj.name}/vendor/glm",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/glew/include"
	}

	libdirs 
	{
		"%{prj.name}/vendor/glew/lib/Release/x64"
	}
	
	links 
	{
		"GLFW",
		"ImGui",
		"opengl32.lib",
		"glew32s.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"LP_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE",
			"GLEW_STATIC"
		}

		filter "configurations:Debug"
			defines "LP_DEBUG"
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

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Lamp/vendor/spdlog/include",
		"Lamp/src",
		"Lamp/vendor/glm",
		"Lamp/vendor",
		"Lamp/vendor/GLFW/include"
	}

	links
	{
		"Lamp"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"LP_PLATFORM_WINDOWS",
		}

		filter "configurations:Debug"
			defines "LP_DEBUG"
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