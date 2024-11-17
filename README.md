This is a very simple Qt desktop application for time tracking.

## Qt installation
- Download pre-built Qt from https://www.qt.io/download-open-source and install in `/$HOME/Qt`
- Build from source:
    - Download from https://download.qt.io/archive/qt/
    - Install in `$HOME/mQt`
    ```bash
    tar xvf qt-everywhere-src-6.8.0.tar.xz
    mkdir -p $HOME/qt qt-everywhere-src-6.8.0/build && cd qt-everywhere-src-6.8.0/build
    ../configure -release -shared -prefix $HOME/mQt -- -DCMAKE_OSX_ARCHITECTURES=arm64
    cmake --build . --parallel
    cmake --install .
    ```


## Build
```bash
# Pre-built Qt on macOS
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/Qt/6.8.0/macos -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_BUILD_TYPE=Release
# Or build from source
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/mQt -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release --parallel
```


## Features
### Start-up
- `00:00` is current working time
- `00:01` is total previous working time + current working time

<img src="demo/start-up.png" width="300">

Note: To start a new working session (ignore previous working time), just create a new `File path` in settings. For example: `/path/to/your/log/name1`, `/path/to/your/log/name2`, ...

### Settings
Default settings are
- Logging working time every `1` minutes
- Capture screenshot every `30` minutes
- Save log and screenshot to `File Path`

<img src="demo/settings.png" width="300">

### Start and end working session
- Click `Start` to start working session

<img src="demo/start.png" width="300">

- `Command + S` to stop working session => give a name for the session

<img src="demo/end.png" width="300">
