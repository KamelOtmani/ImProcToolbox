-- premake5.lua
workspace "ImProcTools"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "ImProcTools"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "WalnutExternal.lua"
include "ImProcProcessing"
include "ImProcTools"