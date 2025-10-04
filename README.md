# ThunderAuto

ThunderAuto is a tool used by FRC teams to create efficient trajectories for autonomous robots to follow. Big changes are in development for the 2026 season, so stay tuned!

<a href="https://apps.microsoft.com/detail/9n2hbs3wm7cm?mode=direct">
	<img src="https://get.microsoft.com/images/en-us%20dark.svg" width="200"/>
</a>



## Building

ThunderAuto uses CMake as its primary build system.
You do not need to install any third-party libraries to build or run ThunderAuto, as it fetches all dependencies automatically at compile time and links statically (in release builds).

### Configuration:
Available options:
- `THUNDERAUTO_DIRECTX11=<ON/OFF>`
  - On Windows, set the graphics backend to DirectX 11 (default on Windows).
- `THUNDERAUTO_OPENGL=<ON/OFF>`
  - Set the graphics backend to OpenGL (default on macOS+Linux).
- `THUNDERLIB_DIR="/path/to/ThunderLib/"`
  - Instead of fetching the latest version of ThunderLib at compile time, specify a local directory where ThunderLib is located (this is useful for simultaneous development of ThunderAuto and ThunderLib).

Generators:
- Windows: `Visual Studio 17 2022` is recommended.
- macOS: `Xcode` is required.
- Linux: Use whatever you like.

Here's an example configuration command for a Windows release build using DirectX 11:
```bash
cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release \
  -DTHUNDER_AUTO_DIRECTX11=ON -G"Visual Studio 17 2022"
```

### Compilation:
You may build the executable with CMake or manually with the generator specified in the configuration step.
```bash
cmake --build build
```
It's not a bad idea to build with multiple threads by adding the flag `--parallel 8` (if you have 8 cores).

### Cross-Compilation for Windows:
As Windows is the primary platform for ThunderAuto users, Windows-specific features need to be regularly tested during development. For macOS and Linux developers, instead of setting up a Windows virtual machine, it is often easier to cross-compile ThunderAuto for Windows using a tool like `mingw-w64`. The built executable can be run using Wine.

> [!NOTE]
> Cross-compilation and Wine emulation are helpful for testing some Windows-specific features, but using a Windows virtual machine is going to work better most of the time.

Developers on macOS or Linux can cross-compile ThunderAuto for Windows with a similar configuration command:

Here's an example configuration command for a Windows x86_64 debug build using OpenGL on macOS using `mingw-64` installed via Homebrew:

```bash
cmake . -Bbuild \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_SYSTEM_VERSION=10.0 \
  -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
  -DCMAKE_C_COMPILER=/opt/homebrew/bin/x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/x86_64-w64-mingw32-g++ \
  -DTHUNDER_AUTO_OPENGL=ON \
  -DTHUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS=ON \
  -G"Unix Makefiles"
```

> [!NOTE]
> On macOS when cross-compiling with OpenGL, if the built executable is intended to be run using Wine on macOS, it is required to add the `-DTHUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS=ON` option to the configuration command. This will ensure that ThunderAuto is built with the correct OpenGL initialization for macOS.

Build the executable normally with CMake:
```bash
cmake --build build
```

To run the built executable using Wine, you can use a similar command:
```bash
WINEPATH=/opt/homebrew/opt/mingw-w64/toolchain-x86_64/x86_64-w64-mingw32/bin/ \
  wine64 build/ThunderAuto.exe
```

