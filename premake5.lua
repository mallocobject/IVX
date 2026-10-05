-- premake5.lua
workspace "Invertix"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Invertix/vendor/GLFW/include"
include "Invertix/vendor/GLFW"

project "Invertix"
    location "Invertix"
    kind "SharedLib"
    language "C++"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "ivx_pch.h"
    pchsource "Invertix/src/ivx_pch.cpp"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "%{prj.name}/src",
        "%{prj.name}/vendor/Elog",
        "%{IncludeDir.GLFW}"
    }

    links { "GLFW", "opengl32.lib", "user32.lib", "gdi32.lib", "shell32.lib" }



    filter "system:windows"
        cppdialect "C++20"
        buildoptions { "/utf-8" }
        staticruntime "On"
        systemversion "latest"
        defines {
            "IVX_PLATFORM_WINDOWS",
            "IVX_BUILD_DLL"
        }

        postbuildcommands {
            "{MKDIR} \"../bin/" .. outputdir .. "/Sandbox\"",
            "{COPY} \"%{cfg.buildtarget.relpath}\" \"../bin/" .. outputdir .. "/Sandbox\""
        }

    filter "configurations:Debug"
        defines "IVX_DEBUG"
        buildoptions "/MDd"
        symbols "on"

    filter "configurations:Release"
        defines "IVX_RELEASE"
        buildoptions "/MD"
        optimize "on"

    filter "configurations:Dist"
        defines "IVX_DIST"
        buildoptions "/MD"
        optimize "on"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "Invertix/src",
        "Invertix/vendor/Elog"
    }

    links { "Invertix" }

    filter "system:windows"
        cppdialect "C++20"
        buildoptions { "/utf-8" }
        staticruntime "On"
        systemversion "latest"
        defines {
            "IVX_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "IVX_DEBUG"
        buildoptions "/MDd"
        symbols "on"

    filter "configurations:Release"
        defines "IVX_RELEASE"
        buildoptions "/MD"
        optimize "on"

    filter "configurations:Dist"
        defines "IVX_DIST"
        buildoptions "/MD"
        optimize "on"