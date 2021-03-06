add_rules("mode.debug", "mode.release")

add_requires("CONAN::boost/1.74.0", { alias = "boost" })
add_requires("CONAN::spdlog/1.8.0", { alias = "spdlog" })
add_requires("CONAN::concurrentqueue/1.0.1", { alias = "concurrentqueue" })
add_requires("CONAN::entt/3.5.1", { alias = "entt" })
add_requires("CONAN::magic_enum/0.6.6", { alias = "magic_enum" })
add_requires("CONAN::nlohmann_json/3.9.1", { alias = "nlohmann_json" })
add_requires("CONAN::glm/0.9.9.8", { alias = "glm" })
add_requires("CONAN::gtest/1.10.0", { alias = "gtest" })
add_requires("CONAN::sol2/3.2.2", { alias = "sol2" })
add_requires("CONAN::libpq/13.1", { alias = "libpq" })

-- This is needed, because for some reason, boost and libpq
-- packages don't link these automatically.
-- TODO: investigate
function add_missing_system_links()
    if is_plat("linux") then
        add_ldflags("-lpthread")
        add_ldflags("-ldl")
    elseif is_plat("windows") then 
        add_links(
            "ws2_32", 
            "secur32", 
            "advapi32", 
            "shell32", 
            "crypt32", 
            "wldap32")
    end
end

target("server")
    set_kind("binary")
    set_languages("cxx20")
    set_rundir("$(projectdir)")
    
    set_pcxxheader("src/server/pch.h")

    add_files(
        "src/server/pch.cpp",
        "src/server/**.cpp")
    add_includedirs("src/server")

    add_missing_system_links()

    add_packages(
        "boost", 
        "spdlog", 
        "concurrentqueue", 
        "entt", 
        "magic_enum", 
        "nlohmann_json", 
        "glm", 
        "gtest", 
        "sol2",
        "libpq")

-- target("server")

target("tests")
    set_kind("binary")
    set_languages("cxx20")
    set_rundir("$(projectdir)")
    
    -- TODO: its own precompiled header ?
    set_pcxxheader("src/server/pch.h")

    -- src files
    add_files(
        "src/server/game/**.cpp",
        "src/server/net/**.cpp",
        "src/server/util/**.cpp",
        "src/server/pch.cpp",
        "src/server/server.cpp",
        "src/test/main.cpp")
    add_includedirs(
        "src/server",
        "src/test")

    add_missing_system_links()
    
    add_packages(
        "boost", 
        "spdlog", 
        "concurrentqueue", 
        "entt", 
        "magic_enum", 
        "nlohmann_json", 
        "glm", 
        "gtest", 
        "sol2",
        "libpq")
-- target("tests")
