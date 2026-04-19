# Joker Mode

A game where you go joker mode.

- Supports hot reloading: updating the code while it's still running!
- Supports hiscores to be posted to an HTTP server
- Everything is cross platform for Windows + Linux

Gameplay very much WIP.

~~Note that the hiscore server is down because I bricked my raspberry pi which was running it. Working on getting it back online.~~
The hiscore server is now running on an ESP32 which I hope is more robust. If you have a ESP32 yourself there are instructions for how to set up the server below.

# TODO
see TODO.md

# Building on Windows

Prerequisites:
- CMake
- Visual Studio (or build tools for MSVC that CMake can detect)
- Windows SDK (Usually installed with Visual Studio)

Please execute the following script within the Developer Command Prompt on Windows Terminal, or any other terminal environment which
has nmake and cl.exe available.
```
build.bat <debug|release>
```

Once you have built once successfully `build_unity.bat` can be run to rebuild the game dll: soc.dll. This is surprisingly soooo much faster
than the incremental build cmake offers. This was added to make hot reloading nearly instant on Windows.

# Building on Linux
Prerequisites:
- CMake
- make
- Working C compiler (that CMake can detect)

```
build.sh <debug|release>
```

# Building via CMake

Prerequisites:
- CMake
- A build generator for your platform (e.g. make, Ninja, Visual Studio)
- Working C compiler (that CMake can detect)

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

# Hot reloading
As long as the data structures have not changed, hot reloading should work by executing the build commands while the
program is running. Try it by looking for the title screen text and changing it, saving and compiling while the game is still running!

# Setting up the ESP32 web server

Prerequisites:
- Platform IO vscode extension
- Some understanding of how to use Platform IO
- A duckdns domain

The ESP32 web server is located under the hiscore_server/ subdirectory. Open this directory as a platform IO project. I tested this with a Devkit v1 ESP32. As long as it's a ESP32 it should work though!

Create a subdirectory in this project called data/ and place two text files in it: config.txt and hiscores.bin. hiscores.bin you can leave empty and config.txt must be in the following format:

```
{WIFI_SSID}
{WIFI_PASSWORD}
{DUCKDNS_URL}
```

Where DUCKDNS_URL is the url that is used to update the IP of your duckdns domain.

Now you're good to build and upload the filesystem and application.

On the game side there's two defines that you need to change:
```
#define HISCORE_SERVER_HOST
#define HISCORE_SERVER_PORT
```
These should be self explanatory.

And you're done!