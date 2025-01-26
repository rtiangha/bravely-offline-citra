#!/bin/bash -ex

if [ "$TARGET" = "appimage-gcc-24.04" ]; then
    # Bundle required QT wayland libraries
    export EXTRA_QT_PLUGINS="waylandcompositor"
    export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so"
else
    # For the linux-fresh verification target, verify compilation without PCH as well.
    export EXTRA_CMAKE_FLAGS=(-DBORKED3DS_USE_PRECOMPILED_HEADERS=OFF)
fi

mkdir build && cd build
cmake .. -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER=gcc-14 \
    -DCMAKE_C_COMPILER=gcc-14 \
    -DCMAKE_CXX_FLAGS="-O2 -lstdc++" \
    -DCMAKE_C_FLAGS="-O2 -lstdc++" \
    -DCMAKE_LINKER_TYPE="MOLD" \
    -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold -lstdc++" \
    -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=mold -lstdc++" \
    "${EXTRA_CMAKE_FLAGS[@]}" \
    -DENABLE_TESTS=ON \
    -DENABLE_QT_TRANSLATION=ON \
    -DUSE_SYSTEM_BOOST=OFF \
    -DUSE_SYSTEM_CATCH2=OFF \
    -DUSE_SYSTEM_CRYPTOPP=OFF \
    -DUSE_SYSTEM_FMT=OFF \
    -DUSE_SYSTEM_XBYAK=OFF \
    -DUSE_SYSTEM_DYNARMIC=OFF \
    -DUSE_SYSTEM_INIH=OFF \
    -DUSE_SYSTEM_FFMPEG_HEADERS=OFF \
    -DUSE_SYSTEM_SOUNDTOUCH=OFF \
    -DUSE_SYSTEM_SDL2=OFF \
    -DUSE_SYSTEM_LIBUSB=OFF \
    -DUSE_SYSTEM_ZSTD=OFF \
    -DUSE_SYSTEM_ENET=OFF \
    -DUSE_SYSTEM_CUBEB=OFF \
    -DUSE_SYSTEM_JSON=OFF \
    -DUSE_SYSTEM_OPENSSL=OFF \
    -DUSE_SYSTEM_CPP_HTTPLIB=OFF \
    -DUSE_SYSTEM_CPP_JWT=OFF \
    -DUSE_SYSTEM_LODEPNG=OFF \
    -DUSE_SYSTEM_OPENAL=OFF \
    -DUSE_SYSTEM_GLSLANG=OFF \
    -DUSE_SYSTEM_VULKAN_HEADERS=OFF \
    -DUSE_SYSTEM_VMA=OFF \
    -DBORKED3DS_USE_EXTERNAL_VULKAN_SPIRV_TOOLS=ON \
    -DBORKED3DS_ENABLE_COMPATIBILITY_REPORTING=OFF \
    -DUSE_DISCORD_PRESENCE=ON
ninja
strip -s bin/Release/*

if [ "$TARGET" = "appimage-gcc-24.04" ]; then
    ninja bundle
    # TODO: Our AppImage environment currently uses an older ccache version without the verbose flag.
    ccache -s
else
    ccache -s -v
fi

ctest -VV -C Release
