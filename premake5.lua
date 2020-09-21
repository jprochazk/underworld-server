

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

function Main()
    include "./conanbuildinfo.premake.lua"

    workspace "Platformer"
    location "./"

    -- runtime == "Debug" or "Release"
    runtime (conan_build_type)
    language "C++"
    cppdialect "C++17"

    conan_basic_setup()

    -- Executable "GameServer"
    project "GameServer"
    kind "ConsoleApp"
    targetdir "bin/"
    objdir "bin/obj"

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
        "-Wextra"
    }

    if conan_build_type == "Debug" then
        symbols "On"
    else
        defines { "NDEBUG" }
        optimize "On"
    end
end

if _ACTION ~= nil and _ACTION ~= "install" and _ACTION ~= "build" then
    Main()
end

