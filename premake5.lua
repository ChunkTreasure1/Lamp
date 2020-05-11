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
include "Lamp/vendor/SoLoud"

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
		"%{prj.name}/vendor/stb_image/stb/**.cpp",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo/**.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo/**.cpp"
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
		"%{prj.name}/vendor/SoLoud/include",
		"%{prj.name}/vendor/rapidxml",
		"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/ImGuizmo"
	}
	
	libdirs
	{
		"%{prj.name}/vendor/assimp"
	}
	
	links 
	{
		"GLFW",
		"ImGui",
		"Glad",
		"SoLoud",
		"xaudio2.lib",
		"opengl32.lib",
		"assimp-vc142-mt.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"LP_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
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
			
project "Sandbox3D"
	location "Sandbox3D"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/engine/**.vert",
		"%{prj.name}/engine/**.frag"
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
		"%{prj.name}/vendor/SoLoud/include",
		"Lamp/vendor/assimp/include"
	}

	libdirs
	{
		"Lamp/vendor/assimp"
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