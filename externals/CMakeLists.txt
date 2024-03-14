# Definitions for all external bundled libraries

# Suppress warnings from external libraries
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W0)
else()
    add_compile_options(-w)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)
include(DownloadExternals)
include(ExternalProject)

# Boost
if (NOT USE_SYSTEM_BOOST)
    message(STATUS "Including vendored Boost library")
    set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/externals/boost" CACHE STRING "")
    set(Boost_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/externals/boost" CACHE STRING "")
    set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "")
    add_library(boost INTERFACE)
    target_include_directories(boost SYSTEM INTERFACE ${Boost_INCLUDE_DIR})

    # Boost::serialization
    file(GLOB boost_serialization_SRC "${CMAKE_SOURCE_DIR}/externals/boost/libs/serialization/src/*.cpp")
    add_library(boost_serialization STATIC ${boost_serialization_SRC})
    target_link_libraries(boost_serialization PUBLIC boost)

    # Boost::iostreams
    add_library(
        boost_iostreams
        STATIC
        ${CMAKE_SOURCE_DIR}/externals/boost/libs/iostreams/src/file_descriptor.cpp
        ${CMAKE_SOURCE_DIR}/externals/boost/libs/iostreams/src/mapped_file.cpp
    )
    target_link_libraries(boost_iostreams PUBLIC boost)
# Add additional boost libs here; remember to ALIAS them in the root CMakeLists!
else()
    unset(BOOST_ROOT CACHE)
    unset(Boost_INCLUDE_DIR CACHE)
    set(Boost_NO_SYSTEM_PATHS OFF CACHE BOOL "" FORCE)
endif()

# Catch2
add_library(catch2 INTERFACE)
if(USE_SYSTEM_CATCH2)
    find_package(Catch2 3.0.0 REQUIRED)
else()
    set(CATCH_INSTALL_DOCS OFF CACHE BOOL "")
    set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "")
    add_subdirectory(catch2)
endif()
target_link_libraries(catch2 INTERFACE Catch2::Catch2WithMain)

# Crypto++
if(USE_SYSTEM_CRYPTOPP)
    find_package(cryptopp REQUIRED)
    add_library(cryptopp INTERFACE)
    target_link_libraries(cryptopp INTERFACE cryptopp::cryptopp)
else()
    if (WIN32 AND NOT MSVC AND "arm64" IN_LIST ARCHITECTURE)
        # TODO: CryptoPP ARM64 ASM does not seem to support Windows unless compiled with MSVC.
        # TODO: See https://github.com/weidai11/cryptopp/issues/1260
        set(CRYPTOPP_DISABLE_ASM ON CACHE BOOL "")
    endif()

    set(CRYPTOPP_BUILD_DOCUMENTATION OFF CACHE BOOL "")
    set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "")
    set(CRYPTOPP_INSTALL OFF CACHE BOOL "")
    set(CRYPTOPP_SOURCES "${CMAKE_SOURCE_DIR}/externals/cryptopp" CACHE STRING "")
endif()

# fmt and Xbyak need to be added before dynarmic
# libfmt
if(USE_SYSTEM_FMT)
    add_library(fmt INTERFACE)
    find_package(fmt REQUIRED)
    target_link_libraries(fmt INTERFACE fmt::fmt)
else()
    option(FMT_INSTALL "" ON)
    add_subdirectory(fmt EXCLUDE_FROM_ALL)
endif()


# Xbyak
if ("x86_64" IN_LIST ARCHITECTURE)
    if(USE_SYSTEM_XBYAK)
        find_package(xbyak REQUIRED)
        add_library(xbyak INTERFACE)
        target_link_libraries(xbyak INTERFACE xbyak::xbyak)
    else()
        add_subdirectory(xbyak EXCLUDE_FROM_ALL)
    endif()
endif()

# Dynarmic
if ("x86_64" IN_LIST ARCHITECTURE OR "arm64" IN_LIST ARCHITECTURE)
    if(USE_SYSTEM_DYNARMIC)
        find_package(dynarmic REQUIRED)
        add_library(dynarmic INTERFACE)
        target_link_libraries(dynarmic INTERFACE dynarmic::dynarmic)
        # The dynarmic package's cmake files are helpfully completely silent
        # so we have to inform the user of its status ourselves
        if(TARGET dynarmic::dynarmic)
            message(STATUS "Found dynarmic")
        endif()
    else()
        set(DYNARMIC_TESTS OFF CACHE BOOL "")
        set(DYNARMIC_FRONTENDS "A32" CACHE STRING "")
        set(DYNARMIC_USE_PRECOMPILED_HEADERS ${CITRA_USE_PRECOMPILED_HEADERS} CACHE BOOL "")
        add_subdirectory(dynarmic EXCLUDE_FROM_ALL)
    endif()
endif()

# getopt
if (MSVC)
    add_subdirectory(getopt)
endif()

# inih
if(USE_SYSTEM_INIH)
    find_package(inih REQUIRED COMPONENTS inih inir)
    add_library(inih INTERFACE)
    target_link_libraries(inih INTERFACE inih::inih inih::inir)
else()
    add_subdirectory(inih)
endif()

# MicroProfile
add_library(microprofile INTERFACE)
target_include_directories(microprofile SYSTEM INTERFACE ./microprofile)

# Nihstro
add_library(nihstro-headers INTERFACE)
target_include_directories(nihstro-headers SYSTEM INTERFACE ./nihstro/include)
if (MSVC)
    # TODO: For some reason MSVC still applies this warning even with /W0 for externals.
    target_compile_options(nihstro-headers INTERFACE /wd4715)
endif()

# Open Source Archives
add_subdirectory(open_source_archives)

# SoundTouch
if(NOT USE_SYSTEM_SOUNDTOUCH)
    set(INTEGER_SAMPLES ON CACHE BOOL "")
    set(SOUNDSTRETCH OFF CACHE BOOL "")
    set(SOUNDTOUCH_DLL OFF CACHE BOOL "")
    add_subdirectory(soundtouch EXCLUDE_FROM_ALL)
    target_compile_definitions(SoundTouch PUBLIC SOUNDTOUCH_INTEGER_SAMPLES)
endif()

# Teakra
if ("x86_64" IN_LIST ARCHITECTURE)
    add_subdirectory(teakra EXCLUDE_FROM_ALL)
else() # use older teakra for arm builds for now
    add_subdirectory(teakra-old teakra EXCLUDE_FROM_ALL)
endif()

# SDL2
if (ENABLE_SDL2 AND NOT USE_SYSTEM_SDL2)
    add_subdirectory(sdl2)
endif()

# libusb
if (ENABLE_LIBUSB AND NOT USE_SYSTEM_LIBUSB)
    add_subdirectory(libusb)
    set(LIBUSB_INCLUDE_DIR "" PARENT_SCOPE)
    set(LIBUSB_LIBRARIES usb PARENT_SCOPE)
endif()

# Zstandard
if(USE_SYSTEM_ZSTD)
    find_package(zstd REQUIRED)
    add_library(zstd INTERFACE)
    if(TARGET zstd::libzstd_shared)
        message(STATUS "Found system Zstandard")
    endif()
    target_link_libraries(zstd INTERFACE zstd::libzstd_shared)
else()
    set(ZSTD_LEGACY_SUPPORT OFF)
    set(ZSTD_BUILD_PROGRAMS OFF)
    set(ZSTD_BUILD_SHARED OFF)
    add_subdirectory(zstd/build/cmake EXCLUDE_FROM_ALL)
    target_include_directories(libzstd_static INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/externals/zstd/lib>)
    add_library(zstd ALIAS libzstd_static)
endif()

# ENet
if(USE_SYSTEM_ENET)
    find_package(libenet REQUIRED)
    add_library(enet INTERFACE)
    target_link_libraries(enet INTERFACE libenet::libenet)
else()
    add_subdirectory(enet)
    target_include_directories(enet INTERFACE ./enet/include)
endif()

# Cubeb
if (ENABLE_CUBEB)
    if(USE_SYSTEM_CUBEB)
        find_package(cubeb REQUIRED)
        add_library(cubeb INTERFACE)
        target_link_libraries(cubeb INTERFACE cubeb::cubeb)
        if(TARGET cubeb::cubeb)
            message(STATUS "Found system cubeb")
        endif()
    else()
        set(BUILD_TESTS OFF CACHE BOOL "")
        set(BUILD_TOOLS OFF CACHE BOOL "")
        set(BUNDLE_SPEEX ON CACHE BOOL "")
        add_subdirectory(cubeb EXCLUDE_FROM_ALL)
    endif()
endif()

# DiscordRPC
if (USE_DISCORD_PRESENCE)
    # rapidjson used by discord-rpc is old and doesn't correctly detect endianness for some platforms.
    include(TestBigEndian)
    test_big_endian(RAPIDJSON_BIG_ENDIAN)
    if(RAPIDJSON_BIG_ENDIAN)
        add_compile_definitions(RAPIDJSON_ENDIAN=1)
    else()
        add_compile_definitions(RAPIDJSON_ENDIAN=0)
    endif()

    # Apply a dummy CLANG_FORMAT_SUFFIX to disable discord-rpc's unnecessary automatic clang-format.
    set(CLANG_FORMAT_SUFFIX "dummy")

    add_subdirectory(discord-rpc EXCLUDE_FROM_ALL)
    target_include_directories(discord-rpc INTERFACE ./discord-rpc/include)
endif()

# JSON
add_library(json-headers INTERFACE)
if (USE_SYSTEM_JSON)
    find_package(nlohmann_json REQUIRED)
    target_link_libraries(json-headers INTERFACE nlohmann_json::nlohmann_json)
    get_target_property(NLOHMANN_PREFIX nlohmann_json::nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)
    # The nlohmann-json3 package expects "#include <nlohmann/json.hpp>"
    # Citra uses "#include <json.hpp>" so we have to add this manually
    target_include_directories(json-headers SYSTEM INTERFACE "${NLOHMANN_PREFIX}/nlohmann")
else()
    target_include_directories(json-headers SYSTEM INTERFACE ./json)
endif()

# OpenSSL
if (USE_SYSTEM_OPENSSL)
    find_package(OpenSSL 1.1)
    if (OPENSSL_FOUND)
        set(OPENSSL_LIBRARIES OpenSSL::SSL OpenSSL::Crypto)
    endif()
endif()

if (NOT OPENSSL_FOUND)
    # LibreSSL
    set(LIBRESSL_SKIP_INSTALL ON CACHE BOOL "")
    set(OPENSSLDIR "/etc/ssl/")
    add_subdirectory(libressl EXCLUDE_FROM_ALL)
    target_include_directories(ssl SYSTEM INTERFACE ./libressl/include)
    target_compile_definitions(ssl PRIVATE -DHAVE_INET_NTOP)
    get_directory_property(OPENSSL_LIBRARIES
        DIRECTORY libressl
        DEFINITION OPENSSL_LIBS)
endif()

# httplib
add_library(httplib INTERFACE)
if(USE_SYSTEM_CPP_HTTPLIB)
    find_package(CppHttp 0.14.1)
    # Detect if system cpphttplib is a shared library
    # this breaks building as Citra relies on functions that are moved
    # into the shared object.
    get_target_property(HTTP_LIBS httplib::httplib INTERFACE_LINK_LIBRARIES)
    if(HTTP_LIBS)
        message(WARNING "Shared cpp-http (${HTTP_LIBS}) not supported. Falling back to bundled...")
        target_include_directories(httplib SYSTEM INTERFACE ./httplib)
    else()
        if(CppHttp_FOUND)
            target_link_libraries(httplib INTERFACE httplib::httplib)
        else()
            message(STATUS "Cpp-httplib not found or not suitable version! Falling back to bundled...")
            target_include_directories(httplib SYSTEM INTERFACE ./httplib)
        endif()
    endif()
else()
    target_include_directories(httplib SYSTEM INTERFACE ./httplib)
endif()
target_compile_options(httplib INTERFACE -DCPPHTTPLIB_OPENSSL_SUPPORT)
target_link_libraries(httplib INTERFACE ${OPENSSL_LIBRARIES})

if (UNIX AND NOT APPLE)
    add_subdirectory(gamemode)
endif()

# cpp-jwt
if (ENABLE_WEB_SERVICE)
    if (USE_SYSTEM_CPP_JWT)
        find_package(cpp-jwt REQUIRED)
        add_library(cpp-jwt INTERFACE)
        target_link_libraries(cpp-jwt INTERFACE cpp-jwt::cpp-jwt)
    else()
        add_library(cpp-jwt INTERFACE)
        target_include_directories(cpp-jwt SYSTEM INTERFACE ./cpp-jwt/include)
        target_compile_definitions(cpp-jwt INTERFACE CPP_JWT_USE_VENDORED_NLOHMANN_JSON)
    endif()
endif()

# lodepng
if(USE_SYSTEM_LODEPNG)
    add_library(lodepng INTERFACE)
    find_package(lodepng REQUIRED)
    target_link_libraries(lodepng INTERFACE lodepng::lodepng)
else()
    add_subdirectory(lodepng)
endif()

# (xperia64): Only use libyuv on Android b/c of build issues on Windows and mandatory JPEG
if(ANDROID)
    # libyuv
    add_subdirectory(libyuv)
    target_include_directories(yuv INTERFACE ./libyuv/include)
endif()

# OpenGL dependencies
if (ENABLE_OPENGL)
    # Glad
    add_subdirectory(glad)
endif()
