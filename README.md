# Custom Citra for Bravely Offline

This is a custom build of Citra originally meant to be used with the [Bravely Offline](https://github.com/osm70/bravely-offline) server/client program by [**osm70**](https://github.com/osm70/bravely-offline) as a drop-in replacement. It can also be used on its own like any normal version of Citra.

This is an experimental build that takes current Citra code and mainly updates most of Citra's third-party dependencies to more modern versions. In that sense, this build is a little more bleeding edge than stock Citra.

There _may_ be performance improvements with this version, but there may also be dragons. If you want to be save, make sure to keep backups of your save games and save states, just in case.

The latest version can be found at the [**Releases**](https://github.com/rtiangha/bravely-offline-citra/releases) page.

## How to Install in Bravely Offline

1. First, back up all the `.exe` and `.dll` files in the `Bravely Offline\DATA\Citra` folder in case you want to return to the old version.

2. Download a version of **Custom Citra for Bravely Offline** from the [Releases](https://github.com/rtiangha/bravely-offline-citra/releases) page.

3. Extract the package.

4. Copy the contents to the `Bravely Offline\DATA\Citra` folder, confirming any files to be overwritten.

5. Launch the app.

To revert to using the original version of Citra that was bundled with Bravely Offline, copy over all of the `.exe` and `.dll` files from your backup into the `Bravely Offline\DATA\Citra` folder  (or, download a copy of [Nightly r1800](https://github.com/rtiangha/bravely-offline-citra/releases/tag/r1800-2022.10.23)), overwriting the files from the new version and relaunch the app.

## How to Compile (GCC or CLANG)

There may be performance differences when compiling with either GCC or CLANG. Here's how to compile with both and you can choose which one performs better.

### MinGW-w64 GCC Build with MSYS2

#### Prerequisites to install

- [MSYS2](https://msys2.github.io/)

Make sure to follow the instructions and update to the latest version by running `pacman -Syu` as many times as needed.

#### Install Citra dependencies for MinGW-w64

- Open the "MSYS2 MinGW 64-bit" (mingw64.exe) shell
- Download and install all dependencies using: `pacman -S mingw-w64-x86_64-{gcc,SDL2,qt6,cmake} make git`

#### Clone the Citra repository with git.

```shell
git clone https://github.com/rtiangha/bravely-offline-citra.git
cd bravely-offline-citra
git submodule update --init --recursive
```

#### Run the following commands to build Citra (static build)

```shell
mkdir build && cd build
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DMICROPROFILE_ENABLED=0" ..
make -jN  (where N = number of CPU threads, ex. -j4)
strip -s bin/Release/*.exe
```

#### Optimizing Builds

If you intend to run Citra on the same computer that you're compiling this on, you may choose to run

`cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native -O2" -DCMAKE_C_FLAGS="-march=native -O2" ..`

before running `make -jN` instead, which may help you eke out a few more fps of performance. Just note that if you choose to compile the program in this way, it may not run on another computer with different specs.

### Clang Build with MSYS2

#### Prerequisites to install

- [MSYS2](https://msys2.github.io/)

Make sure to follow the instructions and update to the latest version by running `pacman -Syu` as many times as needed.

#### Install Citra dependencies

- Open the "MSYS2 Clang64" (clang64.exe) shell
- Download and install all dependencies using: `pacman -S mingw-w64-clang-x86_64-{gcc,qt6,cmake} make git`

#### Clone the Citra repository with git.

```shell
git clone https://github.com/rtiangha/bravely-offline-citra.git
cd bravely-citra-offline 
git submodule update --init --recursive
```

#### Run the following commands to build Citra (static build)

```shell
mkdir build && cd build
cmake -G -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DMICROPROFILE_ENABLED=0" -DMINGW_STATIC_BUILD=1 ..
cmake --build . --config Release -- -j N (where N = number of CPU threads, ex. -j4)
strip -s bin/Release/*.exe
```

#### Optimizing Builds

If you intend to run Citra on the same computer that you're compiling this on, you may choose to run

`cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native -O2" -DCMAKE_C_FLAGS="-march=native -O2" ..`

before running `cmake --build . --config Release -- -j N` instead, which may help you eke out a few more fps of performance. Just note that if you choose to compile the program in this way, it may not run on another computer with different specs.

### Installation in Bravely Offline

If the compilation is successful, the resulting **Custom Citra for Bravely Offline** `.exe` and `.dll` files will be found in the `build/bin/Release` folder. Copy these files to the `Bravely Offline\DATA\Citra` folder, but make sure to back up the original copies first in case you want to return to them later (or download [Nightly r1800](https://github.com/rtiangha/bravely-offline-citra/releases/tag/r1800-2022.10.23)).

## Troubleshooting

* **Issue**:  Citra crashes as soon as I launch the game.

    * This sometimes happens when you switch between Citra versions. To fix it, navigate to the `Bravely Offline\DATA\Citra\user\shaders\opengl` folder, delete all of its contents, then restart the app.

* **Issue**:  When launching Citra, an error appears about missing `libcrypto-3.x64.dll` or `libssl-3-x64.dll` files.

    * If these files didn't already come bundled with your custom version of Citra, you can copy over the files provided from [this installation](https://slproweb.com/download/Win64OpenSSL_Light-3_2_1.exe) of OpenSSL for Windows (i.e. install the program, navigate to its installation folder and copy over the two missing .dll files to the `Bravely Offline\DATA\Citra` folder, and then uninstall OpenSSL). You may also find copies of these files in the [dll](https://github.com/rtiangha/bravely-offline-citra/tree/bravely-offline/dlls) folder of the source code.
 
* **Issue:**:  Game control buttons (ex. keyboard, gamepad, etc.) no longer work.

    * Navigate to `Emulation -> Configure -> Controls` and remap your buttons, even if they look correct at first glance. Button presses need to be registered again in order for things to work properly.
