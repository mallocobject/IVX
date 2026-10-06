project "Glad"
	kind "StaticLib"
	language "C"
	cdialect "C17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/glad/glad.h",
		"include/KHR/khrplatform.h",
		"src/glad.c"
	}

	includedirs { "include" }

	filter "system:windows"
		systemversion "latest"

	filter { "configurations:Debug" }
		runtime "Debug"

	filter { "configurations:Release" }
		runtime "Release"

