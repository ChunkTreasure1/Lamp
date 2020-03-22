project "SoLoud"
    kind "StaticLib"
    language "C++"
    staticruntime "on"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.cpp",
        "src/**.h"
    }

    includedirs
    {
        "include"
    }
    
    filter "system:windows"
        systemversion "latest"
		defines "WITH_XAUDIO2"

    filter "configurations:Debug"
		defines "WITH_XAUDIO2"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
		defines "WITH_XAUDIO2"
        runtime "Release"
        optimize "on"
