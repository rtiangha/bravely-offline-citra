set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

add_executable(borked3ds
    borked3ds.cpp
    borked3ds.rc
    config.cpp
    config.h
    default_ini.h
    emu_window/emu_window_sdl2.cpp
    emu_window/emu_window_sdl2.h
    precompiled_headers.h
    resource.h
)

set_target_properties(borked3ds PROPERTIES OUTPUT_NAME "borked3ds-cli")

if (ENABLE_SOFTWARE_RENDERER)
    target_sources(borked3ds PRIVATE
        emu_window/emu_window_sdl2_sw.cpp
        emu_window/emu_window_sdl2_sw.h
    )
endif()
if (ENABLE_OPENGL)
    target_sources(borked3ds PRIVATE
        emu_window/emu_window_sdl2_gl.cpp
        emu_window/emu_window_sdl2_gl.h
    )
endif()
if (ENABLE_VULKAN)
    target_sources(borked3ds PRIVATE
        emu_window/emu_window_sdl2_vk.cpp
        emu_window/emu_window_sdl2_vk.h
    )
endif()

if (MSVC AND ENABLE_LTO)
  target_compile_options(borked3ds PRIVATE
    /wd5049 # 'string': Embedding a full path may result in machine-dependent output (breaks LTO on MSVC)
  )
endif()

create_target_directory_groups(borked3ds)

target_link_libraries(borked3ds PRIVATE borked3ds_common borked3ds_core input_common network)
target_link_libraries(borked3ds PRIVATE inih)
if (MSVC)
    target_link_libraries(borked3ds PRIVATE getopt)
endif()
target_link_libraries(borked3ds PRIVATE ${PLATFORM_LIBRARIES} SDL2::SDL2 SDL3-shared Threads::Threads)

if (ENABLE_OPENGL)
    target_link_libraries(borked3ds PRIVATE glad)
endif()

if(UNIX AND NOT APPLE)
    install(TARGETS borked3ds RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
endif()

if (BORKED3DS_USE_PRECOMPILED_HEADERS)
    target_precompile_headers(borked3ds PRIVATE precompiled_headers.h)
endif()

# Bundle in-place on MSVC so dependencies can be resolved by builds.
if (MSVC)
    include(BundleTarget)
    bundle_target_in_place(borked3ds)
endif()
