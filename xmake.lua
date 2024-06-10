set_project("HighLevelPuzzle")

add_rules("mode.debug", "mode.release")
add_requires("glfw", "stb", "glm", "glad")
add_requires("imgui", {configs = { glfw = true, opengl3 = true }})

if is_mode("debug") then
    add_defines("MY_DEBUG")
end

target("HLP-Demo")
    set_languages("c99", "cxx20")
    set_kind("binary")
    set_warnings("all")

    add_files("src/**.cpp")
    add_includedirs("src")
    add_packages("glfw", "stb", "glm", "imgui", "glad")

    after_build(function (target)
        os.cp(target:targetfile(), "bin/")
        os.cp("shaders", "bin/")
        os.cp("resources", "bin/")
    end)
target_end()
