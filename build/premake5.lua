
workspace "Sanyan Signals And Slots"
	filename "Sanyan"
	location "../"
	configurations{ "Debug", "Release" }
	objdir "../build"


	project "SanyanLib"
		location "../"
		kind "SharedLib"
		architecture "x64"
		language "C++"
		flags{ "MultiProcessorCompile", "NoIncrementalLink", "RelativeLinks", "FatalLinkWarnings", "FatalCompileWarnings" }
		cppdialect "C++11"
		files{ "../src/**.cpp", "../include/**.hpp"}
		excludes{}
		includedirs{"../include"}
		libdirs{}
		objdir "../build"
		targetdir "../bin/$(Configuration)"

		filter "configurations:Debug"
			optimize "Debug"
			targetname "SanyanD"
		filter "configurations:Release"
			optimize "Speed"
			targetname "Sanyan"
		filter{}

	project "SanyanUnitTest"
		location "../"
		kind "ConsoleApp"
		architecture "x64"
		language "C++"
		flags{ "MultiProcessorCompile", "NoIncrementalLink", "RelativeLinks", "FatalLinkWarnings", "FatalCompileWarnings" }
		cppdialect "C++11"
		files{ "../src/**.cpp", "../include/**.hpp", "../unit_test/**.cpp"}
		excludes{}
		includedirs{"../include"}
		libdirs{}
		objdir "../build"
		targetdir "../bin/$(Configuration)"

		filter "configurations:Debug"
			symbols "On"
			optimize "Debug"
			targetname "SanyanUnitTestD"
		filter "configurations:Release"
			optimize "Speed"
			symbols "Off"
			targetname "SanyanUnitTest"
		filter{}
