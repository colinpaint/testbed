cmake_minimum_required (VERSION 3.16)
#
option (BUILD_FILESYSTEM "Build filesystem" ON)
option (BUILD_VALIB "Build valib" ON)
option (BUILD_SHELLLIB "Build shellLib" ON)
option (BUILD_SHELL "Build shell" ON)
option (BUILD_SIMPLE "Build simple" ON)
option (BUILD_VAINFO "Build vainfo" ON)
option (BUILD_PUTSURFACE "Build putsurface" ON)
#
option (BUILD_AVX "Build avx" ON)
option (BUILD_OPTIMISED "Build optimised" ON)
option (BUILD_LESS_WARNING "Build less warning" ON)
#

project (utils)
  file (GLOB HEADER_FILES utils/*.h)
  file (GLOB SOURCE_FILES utils/*.cpp utils/*.c)
  add_library (${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

  if (MSVC)
    target_compile_options (${PROJECT_NAME} PUBLIC /W4 /fp:fast /std:c++17)
    if (BUILD_AVX)
      target_compile_options (${PROJECT_NAME} PUBLIC /arch:AVX)
    endif()
  else()
    target_compile_definitions (${PROJECT_NAME} PUBLIC _LARGEFILE64_SOURCE _FILE_OFFSET_BITS=64)
    target_compile_options (${PROJECT_NAME} PUBLIC -Wall -Wextra
                                                   -Wno-missing-field-initializers
                                                   -Wno-format-security -Wno-format-overflow
                                                   -pedantic
                                                   -march=native -flax-vector-conversions -ftree-vectorize
                                                   -std=c++17 )
    if (BUILD_LESS_WARNING)
      target_compile_options (${PROJECT_NAME} PUBLIC -Wno-unused-parameter -Wno-sign-compare)
    endif()
    if (BUILD_OPTIMISED)
      target_compile_options (${PROJECT_NAME} PUBLIC -Ofast)
    else()
      target_compile_options (${PROJECT_NAME} PUBLIC -g -O0)
    endif()
endif()

# filesystem app
if (BUILD_FILESYSTEM)
  project (filesystem)
    file (GLOB HEADER_FILES *.h utils/*.h)
    file (GLOB SOURCE_FILES filesystem/main.cpp)
    add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

    target_link_libraries (${PROJECT_NAME} PRIVATE utils)
    if (NOT MSVC)
      target_link_libraries (${PROJECT_NAME} PRIVATE stdc++fs)
    endif()
endif()

# valib lib
if (BUILD_VALIB)
  project (valib)
    if (MSVC)
    else()
      file (GLOB HEADER_FILES valib/*.h utils/*.h)
      file (GLOB SOURCE_FILES valib/*.c)
      add_library (${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

      find_package (PkgConfig REQUIRED)
      pkg_check_modules (X11 REQUIRED IMPORTED_TARGET x11)
      pkg_check_modules (WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
      pkg_check_modules (LIBVA REQUIRED IMPORTED_TARGET libva)
      pkg_check_modules (LIBVA_DRM REQUIRED IMPORTED_TARGET libva-drm)
      pkg_check_modules (LIBVA_X11 REQUIRED IMPORTED_TARGET libva-x11)
      pkg_check_modules (LIBVA_WAYLAND REQUIRED IMPORTED_TARGET libva-wayland)
      target_include_directories (${PROJECT_NAME} PUBLIC PkgConfig::LIBVA PkgConfig::LIBVA_DRM
                                                         PkgConfig::LIBVA_X11 PkgConfig::X11
                                                         PkgConfig::LIBVA_WAYLAND PkgConfig::WAYLAND)
      target_link_libraries (${PROJECT_NAME} PUBLIC utils PkgConfig::LIBVA PkgConfig::LIBVA_DRM
                                                          PkgConfig::LIBVA_X11 PkgConfig::X11
                                                          PkgConfig::LIBVA_WAYLAND PkgConfig::WAYLAND)
    endif()
endif()

# shell lib
if (BUILD_SHELLLIB)
  project (shellLib)
    if (MSVC)
    else()
      file (GLOB HEADER_FILES shellLib/xdg-shell-client-protocol.h)
      file (GLOB SOURCE_FILES shellLib/xdg-shell-protocol.c)
      add_library (${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

      find_package (PkgConfig REQUIRED)
      pkg_check_modules (WAYLAND REQUIRED IMPORTED_TARGET wayland-protocols)
      pkg_check_modules (WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
      #pkg_check_modules (WAYLAND_SCANNER REQUIRED IMPORTED_TARGET wayland-scanner)
      target_include_directories (${PROJECT_NAME} PUBLIC PkgConfig::WAYLAND PkgConfig::WAYLAND_CLIENT)
      target_link_libraries (${PROJECT_NAME} PUBLIC utils PkgConfig::WAYLAND PkgConfig::WAYLAND_CLIENT)

    endif()
endif()

# shell app
if (BUILD_SHELL)
  project (shell)
    if (MSVC)
    else()
      file (GLOB HEADER_FILES shellLib/xdg-shell-client-protocol.h)
      file (GLOB SOURCE_FILES shell/main.c)
      add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
      target_link_libraries (${PROJECT_NAME} PUBLIC utils shellLib)
    endif()
endif()

# simple app
if (BUILD_SIMPLE)
  project (simple)
    if (MSVC)
    else()
      file (GLOB HEADER_FILES shellLib/xdg-shell-client-protocol.h)
      file (GLOB SOURCE_FILES shell/simple.c)
      add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
      target_link_libraries (${PROJECT_NAME} PUBLIC utils shellLib)
    endif()
endif()

# vainfo app
if (BUILD_VAINFO)
  project (vainfo)
    if (MSVC)
    else()
      file (GLOB HEADER_FILES valib/*.h utils/*.h)
      file (GLOB SOURCE_FILES vainfo/vainfo.c)
      add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
      target_link_libraries (${PROJECT_NAME} PUBLIC utils valib)
    endif()
endif()

# putsurface app
if (BUILD_PUTSURFACE)
  project (putsurface)
    if (MSVC)
    else()
      file (GLOB HEADER_FILES valib/*.h utils/*.h)
      file (GLOB SOURCE_FILES putsurface/putsurface.c)
      add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
      target_link_libraries (${PROJECT_NAME} PUBLIC utils valib)
    endif()
endif()
