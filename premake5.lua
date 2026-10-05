-- premake5.lua
workspace "Invertix"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Invertix/vendor/GLFW/include"
IncludeDir["Glad"] = "Invertix/vendor/Glad/include"
IncludeDir["ImGui"] = "Invertix/vendor/imgui"

include "Invertix/vendor/GLFW"
include "Invertix/vendor/Glad"
include "Invertix/vendor/imgui"

project "Invertix"
    location "Invertix"
    kind "SharedLib"
    language "C++"
    staticruntime "off"

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
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}"
    }

    links { "GLFW", 
            "Glad", 
            "ImGui",
            "opengl32.lib", 
            "user32.lib", 
            "gdi32.lib", 
            "shell32.lib" }



    filter "system:windows"
        cppdialect "C++20"
        buildoptions { "/utf-8" }
        systemversion "latest"
        defines {
            "IVX_PLATFORM_WINDOWS",
            "IVX_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }

        postbuildcommands {
            ("IF NOT EXIST \"../bin/" .. outputdir .. "/Sandbox\" mkdir \"../bin/" .. outputdir .. "/Sandbox\""),
            ("{COPY} \"%{cfg.buildtarget.relpath}\" \"../bin/" .. outputdir .. "/Sandbox\"")
        }

    filter "configurations:Debug"
        defines "IVX_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "IVX_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "IVX_DIST"
        runtime "Release"
        optimize "on"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    staticruntime "off"

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
        systemversion "latest"
        defines {
            "IVX_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "IVX_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "IVX_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "IVX_DIST"
        runtime "Release"
        optimize "on"