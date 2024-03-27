# Custom Citra for Bravely Offline

This is a custom build of Citra originally meant to be used with the [Bravely Offline](https://github.com/osm70/bravely-offline) server/client program by [**osm70**](https://github.com/osm70/bravely-offline) as a drop-in replacement. It can also be used on its own like any normal version of Citra.

This is an **experimental** build that takes current Citra code and updates most of Citra's third-party dependencies to more modern versions. In that sense, this build is a little more bleeding edge than stock Citra.

There _may_ be performance improvements with this version, but there may also be dragons. If you want to be safe, make sure to keep backups of your save games and save states, just in case.

The latest version can be found on the [**Releases**](https://github.com/rtiangha/bravely-offline-citra/releases) page.

## How to Install in Bravely Offline

1. First, back up all the `.exe` and `.dll` files in the `Bravely Offline\DATA\Citra` folder in case you want to return to the old version.

2. Download a version of Citra from the [Releases](https://github.com/rtiangha/bravely-offline-citra/releases) page.

3. Extract the package.

4. Copy the contents to the `Bravely Offline\DATA\Citra` folder, confirming any files to be overwritten.

5. Launch the app.

## How to Compile (GCC or CLANG)

There may be performance differences when compiling with either GCC or CLANG. Here's how to compile with both and you can choose which one performs better.

### MinGW-w64 GCC Build with MSYS2

#### Prerequisites to install

- [MSYS2](https://msys2.github.io/)

Make sure to follow the instructions and update to the latest version by running `pacman -Syu` as many times as needed.

#### Install Citra dependencies for MinGW-w64

- Open the "MSYS2 MinGW 64-bit" (mingw64.exe) shell
- Download and install all dependencies using: `pacman -S mingw-w64-x86_64-{gcc,SDL2,qt6,cmake,ninja,spirv-tools} make git`

#### Clone the Citra repository with git.

```shell
git clone https://github.com/rtiangha/bravely-offline-citra.git
cd bravely-offline-citra
git submodule update --init --recursive
```

#### Run the following commands to build Citra

```shell
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
ninja bundle
strip -s bundle/*.exe
```

When complete, all the binaries will be found in `build/bundle`.

#### Optimizing GCC Builds

If you intend to run Citra on the same computer that you're compiling this on, you may choose to run

`cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native -O2" -DCMAKE_C_FLAGS="-march=native -O2" ..`

before running `ninja` instead, which may help you eke out a few more fps of performance. Just note that if you choose to compile the program in this way, it may not run on another computer with different specs.

### Clang Build with MSYS2

#### Prerequisites to install

- [MSYS2](https://msys2.github.io/)

Make sure to follow the instructions and update to the latest version by running `pacman -Syu` as many times as needed.

#### Install Citra dependencies

- Open the "MSYS2 Clang64" (clang64.exe) shell
- Download and install all dependencies using: `pacman -S mingw-w64-clang-x86_64-{gcc,qt6,cmake} mingw-w64-x86_64-{ninja,spirv-tools} make git`

#### Clone the Citra repository with git.

```shell
git clone https://github.com/rtiangha/bravely-offline-citra.git
cd bravely-offline-citra
git submodule update --init --recursive
```

#### Run the following commands to build Citra

```shell
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
ninja bundle
strip -s bundle/*.exe
```

When complete, all the binaries will be found in `build/bundle`.

#### Optimizing Clang Builds

If you intend to run Citra on the same computer that you're compiling this on, you may choose to run

`cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native -O2" -DCMAKE_C_FLAGS="-march=native -O2" ..`

before running `ninja` instead, which may help you eke out a few more fps of performance. Just note that if you choose to compile the program in this way, it may not run on another computer with different specs.

### Installation in Bravely Offline

If the compilation is successful, the resulting **Custom Citra for Bravely Offline** `.exe` and `.dll` files will be found in the `build/bin/Release` folder. Copy these files to the `Bravely Offline\DATA\Citra` folder, but make sure to back up the original copies first in case you want to return to them later (or download [Nightly r1800](https://github.com/rtiangha/bravely-offline-citra/releases/tag/r1800-2022.10.23), the version originally bundled with Bravely Offline).

## Troubleshooting

* **Issue**:  Citra crashes as soon as I launch the game.

    * This sometimes happens when you switch between Citra versions. To fix it, navigate to the `Bravely Offline\DATA\Citra\user\shaders\` folder, delete all of its contents, then restart the app.

* **Issue**:  When launching Citra after compiling under msys2, an error appears about missing DLLs or symbols.

    * If you already have those DLL files in your Citra directory but Citra still thinks they're missing, then this error happens when you use mismatched DLL files (for example, the DLL files linked to citra.exe are newer or different than the ones you already have). You'll need to copy over the DLLs used in msys2 during compilation and overwrite the ones already in your Citra directory. Those DLLs can be found in either the `msys2\<environment>\bin` or `msys2\<environment>\share\qt6\plugins` directories, where `<environment>` is the shell you performed your compilation under (so `clang64` for clang, or `mingw64` for gcc). 
 
* **Issue**:  Game control buttons (ex. keyboard, gamepad, etc.) no longer work.

    * Navigate to `Emulation -> Configure -> Controls` and remap your buttons, even if they look correct at first glance. Button presses need to be registered again in order for things to work properly.

* **Issue**:  I hate this new version of Citra. I want to go back to the old version!

    * To revert to using the original version of Citra that was bundled with Bravely Offline, copy over all of the `.exe` and `.dll` files from your backup into the `Bravely Offline\DATA\Citra` folder, or if you don't have a backup, download a copy of [Nightly r1800](https://github.com/rtiangha/bravely-offline-citra/releases/tag/r1800-2022.10.23) from the Releases page, making sure to overwrite the files that already exist. Then, relaunch the Bravely Offline.

