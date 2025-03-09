# Advanced Auto Clicker

A powerful and customizable auto clicker application with a modern user interface built using C++ and Dear ImGui.

## Features

- 🎯 Precise clicking at specified or current cursor position
- ⌨️ Customizable global hotkeys with modifier support (Ctrl, Alt, Shift)
- ⚡ Adjustable click intervals (1ms - 5000ms)
- 🖱️ Support for both left and right mouse clicks
- 🎨 Modern and sleek dark theme interface
- 🔄 Dynamic cursor position tracking
- 💻 Lightweight and efficient performance

## System Requirements

- Windows 10/11
- DirectX 11 compatible graphics card
- Visual C++ Redistributable 2019 or newer

## Building from Source

### Prerequisites

- Visual Studio 2019 or newer with C++ development tools
- CMake 3.10 or newer
- Git (for cloning)

### Build Steps

1. Clone the repository
```bash
git clone https://github.com/eadrick0/advanced-auto-clicker.git
cd advanced-auto-clicker
```

2. Create build directory and generate project files
```bash
mkdir build
cd build
cmake ..
```

3. Build the project
```bash
cmake --build . --config Release
```

The executable will be located in `build/Release/auto_clicker.exe`

## Usage

1. Launch the application
2. Configure your desired click settings:
   - Set click interval (in milliseconds)
   - Choose between left or right click
   - Enable/disable current cursor position tracking
   - Set custom hotkeys
3. Press the Start button or use your configured hotkey to begin clicking
4. Use the same hotkey or press Stop to halt the clicking operation

## Configuration Options

- **Click Interval**: Adjust the time between clicks (1-5000ms)
- **Click Type**: Choose between left or right mouse button
- **Position Mode**: 
  - Current Position: Follows your cursor
  - Fixed Position: Clicks at a specified coordinate
- **Hotkey Settings**: 
  - Customizable key combinations
  - Support for modifier keys (Ctrl, Alt, Shift)

## Key Bindings

- Default activation key: F6
- Modifier keys can be combined (e.g., Ctrl + Alt + K)
- All key bindings can be customized through the interface

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate Mode Graphical User Interface
- DirectX 11 for rendering
- Windows API for system integration

## Disclaimer

This tool is intended for legitimate automation purposes only. Users are responsible for compliance with applicable laws and regulations when using this software.