

-- .---------. --
-- |  UTILS  | --
-- ^---------^ --

-- according to this http://www.lua.org/manual/5.3/manual.html#pdf-package.config
-- the most reliable way to detect if we are on windows, or other OS
function IsWindows()
	return package.config:sub(1,1) == '\\'
end

-- Returns true if `file` exists
function Exists(file)
    local ok, err, code = os.rename(file, file)
    if not ok then
        -- Permission denied, but it exists
        if code == 13 then
            return true
        end
    end
    return ok, err
end

--- Check if a directory exists in this path
function IsDir(path)
   return Exists(path.."/")
end

-- Changes first letter of string to uppercase
-- e.g. "test" -> "Test"
function TitleCase(s)
	return s:sub(1,1):upper()..s:sub(2)
end

-- premake5 cli options are dumped to a global _OPTIONS table

function RemoveDir(file)
	if not IsDir(file) then return end

    print ("Deleting " .. file .. "...")
	if IsWindows() then
		os.execute("rmdir \"" .. file .. "\" /s /q")
	else
		os.execute("rm -rf \"" .. file .. "\"")
	end
end

function RemoveFile(file)
    if not Exists(file) then return end

    print ("Deleting " .. file .. "...")
	if IsWindows() then
		os.execute("del \"" .. file .. "\"")
	else
		os.execute("rm \"" .. file .. "\"")
	end
end

-- this function returns the option as a string or a default value
function GetOptionStrDefault(name, default)
	local opt = _OPTIONS[name]
	if opt ~= nil then
		return tostring(opt)
    else
        print("No value provided for option " .. name .. ", defaulting to " .. default)
		return tostring(default)
	end
end

-- this function returns true if _OPTIONS[name] exists
function HasOption(name)
	return _OPTIONS[name] ~= nil
end

-- exit with a message
function Exit(code, message)
    if message ~= nil then print(message) end
    os.exit(code or 0)
end

-- .-----------. --
-- |  ACTIONS  | --
-- ^-----------^ --

function Help()
    Exit(0,
"\
Usage: premake5 [options] action\
\
Actions:\
    help - Displays this message.\
    clean - Cleans generated conan & premake files.\
    gmake2 - Generate GNU makefiles for POSIX, MinGW, and Cygwin\
        --cc=VALUE          Choose a C/C++ compiler\
            clang           Clang (clang)\
            gcc             GNU GCC (gcc/g++)\
    install - Fetches and builds dependencies using conan.\
        --build=MODE        Choose a build mode\
            debug           Debug symbols\
            release         Optimization\
        --profile=PROFILE   Which conan profile to use when fetching dependencies.\
                            Conan profiles can be used to store settings such as\
                            which C++ compiler to use, which version of the\
                            STL to use, etc, when fetching dependencies.\
Other options:\
    --verbose                Generate extra debug text output\
    --version                Display version information\
")
end

newaction {
    trigger     = "help",
    description = "Displays this message.",
    execute     = Help
}

function CheckConan()
    -- Check for conan
    local success = os.execute("conan --version > " .. os.tmpname())
    if not success then
        Exit(1, "Please install Conan - https://conan.io/")
    end
end

function Install()
    CheckConan()
    print("Installing...")
    local buildType = GetOptionStrDefault("build", "release")
    local profile = GetOptionStrDefault("profile", "default")
    local success = os.execute("conan install . -s build_type=" .. TitleCase(buildType) .. " --build=missing --profile=" .. profile)
    if not success then
        Exit(1, "Installation failed, see above output for details.")
    else
        Exit()
    end
end

newoption {
    trigger     = "build",
    value       = "MODE",
    description = "Build mode",
    allowed = {
        { "release", "Optimization, no debug symbols" },
        { "debug",   "No optimization, debug symbols" },
    }
}

newoption {
    trigger     = "profile",
    value       = "CONAN_PROFILE",
    description = "Which conan profile to use"
}

newaction {
    trigger     = "install",
    description = "Fetches and builds dependencies using conan.",
    execute     = Install
}

function Clean()
    if Exists("Makefile") then os.execute("make clean") end
    local files = {"Makefile", "graph_info.json", "server.make", "server", "tests.make", "tests", "conaninfo.txt", "conanbuildinfo.txt", "conanbuildinfo.premake.lua", "conan.lock", "compile_commands.json"}
    for _, file in ipairs(files) do
        RemoveFile(file)
    end

    local dirs = {"bin", "obj"}
    for _, dir in ipairs(dirs) do
        RemoveDir(dir)
    end
end

newaction {
    trigger = "clean",
    description = "Cleans generated conan & premake files.",
    execute = Clean
}

function Common()
    include "./conanbuildinfo.premake.lua"

    workspace "Binary"
    location "./"

    -- runtime == "Debug" or "Release"
    runtime (conan_build_type)
    language "C++"
    cppdialect "C++17"

    conan_basic_setup()
end

function Binary()
    -- Executable "server"
    project "server"
    kind "ConsoleApp"
    targetdir "./"
    objdir "./obj"

    files {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs {
        "src",
        "vendor"
    }

    buildoptions {
        "-Wall",
        "-Wextra",
        "-Wno-unused-parameter"
    }

    if conan_build_type == "Debug" then
        symbols "On"
    else
        defines { "NDEBUG" }
        optimize "Speed"
    end
end

function Tests()
    -- Executable "tests"
    project "tests"
    kind "ConsoleApp"
    targetdir "./"
    objdir "./obj"

    files {
        "src/**.h",
        "src/**.cpp",
        "test/**.cpp"
    }

    excludes {
        "src/main.cpp"
    }

    includedirs {
        "src",
        "test",
        "vendor"
    }

    buildoptions {
        "-Wall",
        "-Wextra",
        "-Wno-unused-parameter"
    }

    if conan_build_type == "Debug" then
        symbols "On"
    else
        defines { "NDEBUG" }
        optimize "Speed"
    end
end

if _ACTION ~= nil and _ACTION ~= "install" and _ACTION ~= "build" and _ACTION ~= "help" and _ACTION ~= "clean" then
    Common()
    Binary()
    Tests()
end

