newoption {
    trigger = "with-shared-lib",
    description = "Enable building shared library."
}

newoption {
    trigger = "with-tools",
    description = "Enable building tools."
}

solution "any-vm"
    configurations { "Debug", "Release" }
    if _ACTION == "xcode4" then
        platforms { "Universal" }
    else
        platforms { "x32", "x64", "Native" }
    end
    configuration { "Debug" }
        defines { "ANY_DEBUG=1" }
    configuration { "Debug and linux-*" }
        buildoptions { "-fsanitize=address" }
        linkoptions { "-lasan" }
    configuration { "gcc or clang" }
        buildoptions_c { "-std=c99" }
    configuration {}
        flags { "ExtraWarnings", "FatalWarnings" }
    startproject "utest"

DIR = {}
DIR.ROOT    = path.getabsolute("../")
DIR.BUILD   = path.join(DIR.ROOT, ".build")
DIR.DEPENDS = path.join(DIR.ROOT, "3rdparty")
DIR.SOURCE  = path.join(DIR.ROOT, "src")
DIR.INCLUDE = path.join(DIR.SOURCE, "inc")
DIR.PRIVATE = path.join(DIR.SOURCE, "private")
DIR.UTEST   = path.join(DIR.SOURCE, "utest")

dofile("toolchain.lua")
if not toolchain(DIR.BUILD, DIR.THIRD_PARTY) then 
    return -- no action specified
end

local project_any = dofile("any.lua")

project_any("StaticLib")

dofile("utest.lua")

if _OPTIONS["with-shared-lib"] then
    group("shared")
    project_any("SharedLib")
end