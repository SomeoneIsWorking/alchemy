cmake_minimum_required(VERSION 3.20)
project(alchemy LANGUAGES C CXX)

# The Alchemy engine layer: asset readers and the input abstraction shared by
# every game built on Gap/Alchemy. It knows nothing about a recompiler, a
# platform loader, or a specific title -- src/ includes only its own headers,
# which is what let it be lifted out of the X-Men Legends II port unchanged.
#
# Consumed by a game port with add_subdirectory(); it also builds standalone,
# which is how its tests run without a game install.

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(PkgConfig QUIET)

# TWO SDLs, on purpose, exactly as the X-Men Legends II port had it.
#
# SDL3 backs the engine's input abstraction, because that is what a live port
# links. SDL2 backs the asset viewers, which are SEPARATE BINARIES -- keeping
# them on SDL2 is what stops two SDLs ever being linked into one process. Do
# not "simplify" these into one: the split is the safety property.
find_package(SDL3 QUIET)
if(NOT SDL3_FOUND AND PkgConfig_FOUND)
    pkg_check_modules(SDL3 QUIET IMPORTED_TARGET sdl3)
endif()
if(TARGET SDL3::SDL3)
    set(ALCHEMY_SDL3 SDL3::SDL3)
elseif(SDL3_FOUND)
    set(ALCHEMY_SDL3 PkgConfig::SDL3)
endif()

find_package(SDL2 QUIET)          # asset viewers only -- see above
if(NOT SDL2_FOUND AND PkgConfig_FOUND)
    pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
endif()
if(TARGET SDL2::SDL2)
    set(ALCHEMY_SDL2 SDL2::SDL2)
elseif(SDL2_FOUND)
    set(ALCHEMY_SDL2 PkgConfig::SDL2)
endif()

# The IGB asset readers: containers, meshes, rasters, and the Enbaya-compressed
# animation codec.
add_library(alchemy
    src/igb.c
    src/igb_image.c
    src/igb_internal.c
    src/igb_mesh.c
    src/igb_object.c
    src/igb_raster.c
    src/igb_anim.c
)
target_include_directories(alchemy PUBLIC src)
target_link_libraries(alchemy PUBLIC m)

# The engine's controller snapshot is platform-neutral. The SDL adapter is a
# peer target because it exposes SDL at its host-facing event boundary; tools
# that only consume engine state must not inherit an SDL dependency.
add_library(alchemy_input
    src/input/controller.cpp
)
add_library(alchemy::input ALIAS alchemy_input)
target_include_directories(alchemy_input PUBLIC include)
target_compile_features(alchemy_input PUBLIC cxx_std_20)
if(ALCHEMY_SDL3)
    add_library(alchemy_input_sdl
        src/input/sdl_controller.cpp
    )
    add_library(alchemy::input_sdl ALIAS alchemy_input_sdl)
    target_include_directories(alchemy_input_sdl PUBLIC include)
    target_compile_features(alchemy_input_sdl PUBLIC cxx_std_20)
    target_link_libraries(alchemy_input_sdl PUBLIC alchemy_input ${ALCHEMY_SDL3})
endif()

# Viewers and dump tools are built only when this is the top-level project. A
# game port consuming the library does not want three extra SDL2 binaries in
# its build, and the viewers stay SDL2 deliberately -- separate binaries, so a
# port on SDL3 never links two SDLs into one process.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    add_executable(igb_dump tools/igb_dump.c)
    target_link_libraries(igb_dump PRIVATE alchemy)

    add_library(alchemy_viewer_config STATIC apps/viewer_config.c)
    target_include_directories(alchemy_viewer_config PUBLIC apps)
    if(ALCHEMY_SDL2)
        foreach(view x2view meshview flyview)
            add_executable(${view} apps/${view}.c)
            target_link_libraries(${view} PRIVATE alchemy alchemy_viewer_config m ${ALCHEMY_SDL2})
        endforeach()
    else()
        message(STATUS "alchemy: SDL2 not found -- the viewers are NOT built. "
                       "The library and its tests are unaffected.")
    endif()

    add_executable(test_enbaya tests/test_enbaya.c)
    target_link_libraries(test_enbaya PRIVATE alchemy)
    target_compile_options(test_enbaya PRIVATE -UNDEBUG)
    add_test(NAME enbaya COMMAND test_enbaya)

    add_executable(test_igb_image tests/test_igb_image.c)
    target_link_libraries(test_igb_image PRIVATE alchemy)
    add_test(NAME igb_image COMMAND test_igb_image)

    add_executable(test_igb_image_real tests/test_igb_image_real.c)
    target_link_libraries(test_igb_image_real PRIVATE alchemy)
    add_test(NAME igb_image_real COMMAND test_igb_image_real)
    set_tests_properties(igb_image_real PROPERTIES
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                         SKIP_RETURN_CODE 77)

    add_executable(test_scene_options tests/test_scene_options.c)
    target_link_libraries(test_scene_options PRIVATE alchemy)
    target_compile_options(test_scene_options PRIVATE -UNDEBUG)
    add_test(NAME scene_options COMMAND test_scene_options)

    add_executable(test_viewer_config tests/test_viewer_config.c)
    target_link_libraries(test_viewer_config PRIVATE alchemy_viewer_config)
    target_compile_options(test_viewer_config PRIVATE -UNDEBUG)
    add_test(NAME viewer_config COMMAND test_viewer_config)

    add_executable(test_input tests/test_input.cpp)
    target_link_libraries(test_input PRIVATE alchemy_input)
    target_compile_options(test_input PRIVATE -UNDEBUG)
    add_test(NAME input COMMAND test_input)

    if(ALCHEMY_SDL3)
        add_executable(test_sdl_input tests/test_sdl_input.cpp)
        target_link_libraries(test_sdl_input PRIVATE alchemy_input_sdl)
        target_compile_options(test_sdl_input PRIVATE -UNDEBUG)
        add_test(NAME sdl_input COMMAND test_sdl_input)
    endif()

    # XMLB is Raven's binary XML container. The selftest builds its own corpus
    # rather than reading shipped game assets, so it needs no install.
    add_test(NAME xmlb_roundtrip
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tools/xmlb.py --selftest)
    add_test(NAME mua_corpus_selftest
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tools/mua_corpus.py --selftest)

    add_test(NAME structure
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_structure.py)
    add_test(NAME structure_selftest
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_structure.py --selftest)
    add_test(NAME python_lint
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tools/lint.py)
    set_tests_properties(python_lint PROPERTIES SKIP_RETURN_CODE 77)

    add_test(NAME cpp_format
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tools/cpp_quality.py format)
    add_test(NAME cpp_tidy
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tools/cpp_quality.py tidy
                     --build-dir ${CMAKE_BINARY_DIR})
    set_tests_properties(cpp_format cpp_tidy PROPERTIES SKIP_RETURN_CODE 77)
endif()
