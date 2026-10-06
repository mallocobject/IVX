-- premake5.lua
workspace "IVX"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["Elog"] = "IVX/vendor/Elog/"
IncludeDir["GLFW"] = "IVX/vendor/GLFW/include"
IncludeDir["Glad"] = "IVX/vendor/Glad/include"
IncludeDir["ImGui"] = "IVX/vendor/imgui"
IncludeDir["glm"] = "IVX/vendor/glm"

include "IVX/vendor/GLFW"
include "IVX/vendor/Glad"
include "IVX/vendor/imgui"

project "IVX"
    location "IVX"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "ivxpch.h"
    pchsource "IVX/src/ivxpch.cpp"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{IncludeDir.Elog}/elog/**.hpp",
        "%{IncludeDir.glm}/glm/**.hpp",
        "%{IncludeDir.glm}/glm/**.inl"
    }

    defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

    includedirs {
        "%{prj.name}/src",
        "%{IncludeDir.Elog}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}"
    }

    links { "GLFW", 
            "Glad", 
            "ImGui",
            "opengl32.lib" }



    filter "system:windows"
        buildoptions { "/utf-8" }
        systemversion "latest"
        defines {
            "IVX_PLATFORM_WINDOWS",
            "IVX_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }


    filter "configurations:Debug"
        defines "IVX_CONFIG_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "IVX_CONFIG_RELEASE"
        runtime "Release"
        optimize "on"


project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }


    includedirs {
        "IVX/src",
        "IVX/vendor"
    }

    links { "IVX" }

    filter "system:windows"
        buildoptions { "/utf-8" }
        systemversion "latest"
        defines {
            "IVX_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "IVX_CONFIG_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "IVX_CONFIG_RELEASE"
        runtime "Release"
        optimize "on"
