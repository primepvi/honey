add_rules("mode.debug", "mode.release")

target("honey")
set_kind("binary")
add_files("src/*.c")
