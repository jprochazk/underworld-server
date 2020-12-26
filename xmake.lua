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
add_requires("CONAN::libpqxx/7.3.0", { alias = "libpqxx" })

-- This is needed, because for some reason, they aren't linked automatically
-- TODO: investigate, submit an issue.
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
    set_languages("cxx17")
    
    set_pcxxheader("src/pch.h")

    add_files(
        "src/pch.cpp",
        "src/**.cpp")
    add_includedirs("src")

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
        "libpqxx")

    set_rundir("$(projectdir)")
-- target("server")

target("tests")
    set_kind("binary")
    set_languages("cxx17")
    
    set_pcxxheader("src/pch.h")

    -- src files
    add_files(
        "src/pch.cpp",
        "src/game/**.cpp",
        "src/net/**.cpp",
        "src/util/**.cpp",
        "src/server.cpp")
    add_includedirs("src")

    -- test files
    add_files("test/main.cpp")
    add_includedirs("test")

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
        "libpqxx")

    set_rundir("$(projectdir)")
-- target("tests")
