# Reproducible source dependencies for hosts without system SDL packages.
include(FetchContent)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(alchemy_sdl2
    URL https://codeload.github.com/libsdl-org/SDL/tar.gz/5d249570393f7a37e037abf22cd6012a4cc56a71
    URL_HASH SHA256=10f1194f8d2e4a73ca1c7c553b3189d0b68ab9ef3544e8d2a268e4429456b373)
FetchContent_MakeAvailable(alchemy_sdl2)
FetchContent_Declare(alchemy_sdl3
    URL https://codeload.github.com/libsdl-org/SDL/tar.gz/683181b47cfabd293e3ea409f838915b8297a4fd
    URL_HASH SHA256=f5b3a3b27ae296da479ab4642c456ff620d268e65d642bdfc934aa210107593f)
FetchContent_MakeAvailable(alchemy_sdl3)
