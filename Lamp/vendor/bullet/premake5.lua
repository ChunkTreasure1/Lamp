project "Bullet"
	location "Bullet"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")
	
	files
	{
		"src/Bullet3Collision/**/**.cpp",
		"src/Bullet3Collision/**/**.h",
		"src/Bullet3Common/**/**.cpp",
		"src/Bullet3Common/**/**.h",
		"src/Bullet3Dynamics/**/**.cpp",
		"src/Bullet3Dynamics/**/**.h",
		"src/Bullet3Geometry/**/**.cpp",
		"src/Bullet3Geometry/**/**.h",
		"src/BulletCollision/**/**.cpp",
		"src/BulletCollision/**/**.h",
		"src/BulletDynamics/**/**.cpp",
		"src/BulletDynamics/**/**.h",
		"src/BulletInverseDynamics/**/**.cpp",
		"src/BulletInverseDynamics/**/**.h",
		"src/BulletSoftBody/**/**.cpp",
		"src/BulletSoftBody/**/**.h",
		"src/LinearMath/**/**.cpp",
		"src/LinearMath/**/**.h",
		"src/btBulletCollisionAll.cpp",
		"src/btBulletCollisionCommon.h",
		"src/btBulletDynamicsAll.cpp",
		"src/btBulletDynamicsCommon.h",
		"src/btLinearMathAll.cpp"
	}
	
	includedirs
	{
		"src"
	}
	
	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
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