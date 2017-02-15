solution "Nature3.0"
	configurations  {"Debug64","Release64"}

project "Nature3.0App"
	kind "ConsoleApp"
	language "C++"
	location "vsfiles"
	files 
	{
		"src/**.cc",
		"src/**.h",
		"depen/imgui/**.cpp"
	}
	includedirs
	{
		"src",
		"depen/glfw/include",
		"depen/glm/glm",
		"depen/glew/include",
		"depen/imgui",
		"depen/stb",
		"depen/tinyobj"
	}
	libdirs
	{
		"depen/glfw/lib",
		"depen/glew/lib/Release/x64"
	}
	links
	{
		"opengl32",
		"glfw3",
		"glew32s"
	}
	flags 
	{

	}

	configuration "Debug64"
		defines
		{
			"_DEBUG"
		}
		platforms "x64"
		targetdir "build/Debug64"
		flags "Symbols"

	configuration "Release64"
		defines
		{
			"NDEBUG"
		}
		platforms "x64"
		targetdir "build/Release64"
		flags "Optimize"

