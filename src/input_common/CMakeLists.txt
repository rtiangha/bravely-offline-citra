add_library(input_common STATIC
    analog_from_button.cpp
    analog_from_button.h
    keyboard.cpp
    keyboard.h
    main.cpp
    main.h
    motion_emu.cpp
    motion_emu.h
    precompiled_headers.h
    touch_from_button.cpp
    touch_from_button.h
    sdl/sdl.cpp
    sdl/sdl.h
    udp/client.cpp
    udp/client.h
    udp/protocol.cpp
    udp/protocol.h
    udp/udp.cpp
    udp/udp.h
)

if(ENABLE_SDL2)
    target_sources(input_common PRIVATE
        sdl/sdl_impl.cpp
        sdl/sdl_impl.h
    )
    target_link_libraries(input_common PRIVATE SDL2::SDL2 SDL3-shared)
    target_compile_definitions(input_common PRIVATE HAVE_SDL2 HAVE_SDL3)
endif()

if(ENABLE_LIBUSB)
    target_sources(input_common PRIVATE
        gcadapter/gc_adapter.cpp
        gcadapter/gc_adapter.h
        gcadapter/gc_poller.cpp
        gcadapter/gc_poller.h
    )
    target_include_directories(input_common PRIVATE ${LIBUSB_INCLUDE_DIR})
    target_link_libraries(input_common PRIVATE ${LIBUSB_LIBRARIES})
    add_definitions(-DENABLE_GCADAPTER)
endif()

if (MSVC AND ENABLE_LTO)
  target_compile_options(input_common PRIVATE
    /wd5049 # 'string': Embedding a full path may result in machine-dependent output (breaks LTO on MSVC)
  )
endif()

create_target_directory_groups(input_common)
target_link_libraries(input_common PUBLIC borked3ds_core PRIVATE borked3ds_common ${Boost_LIBRARIES})

if (BORKED3DS_USE_PRECOMPILED_HEADERS)
    target_precompile_headers(input_common PRIVATE precompiled_headers.h)
endif()
