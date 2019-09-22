workspace "Lamp"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Lamp"
	location "Lamp"
	kind "StaticLib"
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

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/include",
	}

	libdirs 
	{
		"%{prj.name}/vendor/glew",
		"%{prj.name}/vendor/glfw"
	}
	
	links 
	{
		"glew32s.lib",
		"opengl32.lib",
		"glfw3.lib"
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
		"Lamp/src",
		"Lamp/vendor/include",
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