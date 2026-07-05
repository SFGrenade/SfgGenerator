# SfgGenerator Codebase Structure

## Headers (`include/`)

### `common/`
Classes that work across plugins:
- `_gui.hpp`: SDL helpers
- `BasePlugin.hpp`: CLAP plugin base class
- `NoteMap.hpp`: Managing active notes
- `SplinePoint.hpp`: Interpolation of points
- `StandardFolders.hpp`: Getting folder paths
- `Timer.hpp`: Native clocks
- `WindowFunctions.hpp`: Windowing functions (e.g., Hann)

### `libraryExtensions/`
Includes and extensions of libraries:
- `_clap.hpp`: Globals and `clap_stream_t` to `std::streambuf` interface
- `_fftw.hpp`: fftw3 and `shared_ptr` creation
- `logging.hpp`: spdlog and fmt and fmt formatters
- `math.hpp`: Simple pow functions

### `main/`
- `main.hpp`: Main CLAP entrypoint

### `organ/`
Special classes for the organ plugin:
- `FluePipe.hpp`
- `Pipe.hpp`
- `ReedPipe.hpp`

### `plugin/`
The CLAP plugin classes:
- `AudioAnalysis.hpp`
- `AudioLerpEffect.hpp`
- `NoiseGenerator.hpp`
- `OrganGenerator.hpp`
- `ParamMultiplex.hpp`

### `widgets/`
UI code building on top of SDL:
- `AudioSampleDisplay.hpp`
- `Bar.hpp`
- `Button.hpp`
- `FrequencyDisplay.hpp`
- `HorizontalDbfsDisplay.hpp`
- `Label.hpp`
- `Slider.hpp`
- `Widget.hpp`

## Source Files (`src/`)

Same subfolder structure as headers:

### `common/`
- `_gui.cpp`
- `BasePlugin.cpp`
- `NoteMap.cpp`
- `SplinePoint.cpp`
- `StandardFolders.cpp`
- `Timer.cpp`
- `WindowFunctions.cpp`

### `libraryExtensions/`
- `_clap.cpp`
- `_fftw.cpp`
- `math.cpp`

### `main/`
- `main.cpp`

### `organ/`
- `FluePipe.cpp`
- `Pipe.cpp`
- `ReedPipe.cpp`

### `plugin/`
- `AudioAnalysis.cpp`
- `AudioLerpEffect.cpp`
- `NoiseGenerator.cpp`
- `OrganGenerator.cpp`
- `ParamMultiplex.cpp`

### `widgets/`
- `AudioSampleDisplay.cpp`
- `Bar.cpp`
- `Button.cpp`
- `FrequencyDisplay.cpp`
- `HorizontalDbfsDisplay.cpp`
- `Label.cpp`
- `Slider.cpp`
- `Widget.cpp`

## Protobuf Files (`proto/`)

### `plugin/`
Contains plugin state:
- `AudioAnalysis.proto`
- `AudioLerpEffect.proto`
- `NoiseGenerator.proto`
- `OrganGenerator.proto`
- `ParamMultiplex.proto`

## Resources (`resources/`)

Currently contains only fonts:
- `fonts/`
  - `NotoSansMono-Regular.ttf`
  - `NotoSerif-Regular.ttf`
  - `Pixels.png`
  - `License.md`
