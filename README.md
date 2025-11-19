# Guitar Tab VST Plugin

A VST plugin for writing and editing guitar tablature directly within your DAW. Store your guitar tabs alongside your project for easy reference!

## Features

- **Visual Tab Editor**: Easy-to-use grid interface for writing tabs
- **Multiple Tunings**: Standard, Drop D, Drop C, 7-string, 8-string, and more
- **Keyboard Navigation**: Fast tab entry using number keys and arrow keys
- **DAW Integration**: Tab data saves with your project
- **Export**: Copy tabs to clipboard in text format

## Keyboard Shortcuts

- **0-9**: Enter fret number
- **Arrow Keys**: Navigate between strings and positions
- **Space**: Move to next column
- **Delete/Backspace**: Clear fret
- **+/=**: Insert column
- **Cmd+-**: Delete column

## Building the Plugin

### Prerequisites

1. **CMake** (3.15 or later)
   ```bash
   brew install cmake
   ```

2. **JUCE Framework** (already in ~/Downloads/JUCE)

3. **Xcode Command Line Tools** (already installed)

### Build Steps

1. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

2. Generate build files:
   ```bash
   cmake ..
   ```

3. Build the plugin:
   ```bash
   cmake --build . --config Release
   ```

4. The plugin will be built in different formats:
   - **VST3**: `build/TabVST_artefacts/Release/VST3/Tab VST.vst3`
   - **AU**: `build/TabVST_artefacts/Release/AU/Tab VST.component`
   - **Standalone**: `build/TabVST_artefacts/Release/Standalone/Tab VST.app`

### Installation

Copy the plugin to your system plugin folder:

**macOS VST3:**
```bash
cp -r build/TabVST_artefacts/Release/VST3/Tab\ VST.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

**macOS AU:**
```bash
cp -r build/TabVST_artefacts/Release/AU/Tab\ VST.component ~/Library/Audio/Plug-Ins/Components/
```

## Usage

1. Load the plugin in your DAW (as an instrument or MIDI FX)
2. Select your guitar tuning from the dropdown
3. Click on the editor to focus it
4. Use keyboard shortcuts to enter your tabs
5. Save your DAW project - the tabs will be saved with it!
6. Use "Copy to Clipboard" to export tabs as text

## Project Structure

```
tab_vst/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
└── Source/
    ├── PluginProcessor.h/cpp    # Main audio processor
    ├── PluginEditor.h/cpp       # Main UI window
    ├── TabEngine.h/cpp          # Tab data model
    └── TabEditorComponent.h/cpp # Grid editor widget
```

## Technical Details

- Built with JUCE 7.x
- Supports VST3, AU, and Standalone formats
- Stores tab data as XML in DAW project state
- Pure GUI plugin (no audio processing)

## Future Enhancements

Potential features to add:
- Scrolling for longer tabs
- Multiple tab pages/songs per instance
- Import/Export Guitar Pro format
- Chord library
- Print preview
- Custom tuning creator
- Rhythm notation
- Playing techniques notation (bends, slides, etc.)

## License

[Add your license here]
