-- premake5.lua
workspace "PiRay"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "PiRay"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "PiRay"