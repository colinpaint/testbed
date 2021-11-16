cmake_minimum_required (VERSION 3.16)
cmake_policy (SET CMP0072 NEW)
#
option (BUILD_FILESYSTEM "Build filesystem" ON)
option (BUILD_WAYLAND "Build wayland" ON)
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

# waylandtest app
if (BUILD_WAYLAND)
  project (wayland)
    if (MSVC)
    else()
      file (GLOB HEADER_FILES wayland/*.h utils/*.h)
      #file (GLOB SOURCE_FILES wayland/putsurfaceWayland.c wayland/vaDisplay.c wayland/vaDisplayWayland.c)
      file (GLOB SOURCE_FILES wayland/vainfo.c
                              wayland/vaDisplay.c
                              wayland/vaDisplayDrm.c wayland/vaDisplayWayland.c wayland/vaDisplayX11.c)
      add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

      find_package (PkgConfig REQUIRED)
      pkg_check_modules (X11 REQUIRED IMPORTED_TARGET x11)
      pkg_check_modules (WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
      pkg_check_modules (LIBVA REQUIRED IMPORTED_TARGET libva)
      pkg_check_modules (LIBVA_DRM REQUIRED IMPORTED_TARGET libva-drm)
      pkg_check_modules (LIBVA_X11 REQUIRED IMPORTED_TARGET libva-x11)
      pkg_check_modules (LIBVA_WAYLAND REQUIRED IMPORTED_TARGET libva-wayland)
      target_include_directories (${PROJECT_NAME} PRIVATE PkgConfig::LIBVA PkgConfig::LIBVA_DRM
                                                          PkgConfig::LIBVA_X11 PkgConfig::X11
                                                          PkgConfig::LIBVA_WAYLAND PkgConfig::WAYLAND)
      target_link_libraries (${PROJECT_NAME} PUBLIC utils PkgConfig::LIBVA PkgConfig::LIBVA_DRM
                                                          PkgConfig::LIBVA_X11 PkgConfig::X11
                                                          PkgConfig::LIBVA_WAYLAND PkgConfig::WAYLAND)
    endif()
endif()
