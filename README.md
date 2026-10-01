[![Build and Release](https://github.com/MADS-NET/rerunner_plugin/actions/workflows/release.yml/badge.svg)](https://github.com/MADS-NET/rerunner_plugin/actions/workflows/release.yml) ![Dynamic JSON Badge](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2FMADS-NET%2F.github%2Fmain%2Fprofile%2Fpackages.json&query=%24.packages.%5B'rerunner.plugin'%5D.type&label=mads%20package&logo=C%2B%2B)


# rerunner plugin for MADS

This is a Sink plugin for [MADS](https://github.com/MADS-NET/MADS) that uses [Rerun](https://www.rerun.io/) to visualize time series data.

The plugin allows you to log time series data to Rerun, where it can be visualized as time series. It supports automatic calculation of the autocorrelation function (ACF) for specified keypaths, making it easier to analyze the temporal dependencies in your data.

> This plugin has been updated for MADS v2

## Supported platforms

Currently, the supported platforms are:

* **Linux** 
* **MacOS**
* **Windows**


## Package install

with MADS v2.1.1 or later, install with:

```bash
mads package --install rerunner.plugin
```


## Install from binaries

Linux and MacOS:

```bash
cmake -Bbuild -DCMAKE_INSTALL_PREFIX="$(mads -p)"
cmake --build build -j4
sudo cmake --install build
```

Windows:

```powershell
cmake -Bbuild -DCMAKE_INSTALL_PREFIX="$(mads -p)"
cmake --build build --config Release
cmake --install build --config Release
```

**Note**: on MacOS with CMake 4.x the Arrow library cannot be compiled. Currently, the only solution is to separately install Arrow via homebrew (`brew install apache-arrow`), then configure the project with the option `-DRERUN_USE_SYSTEM_ARROW=ON`.

**Note**: on Windows and with CMake versions newer than 4.0, some of the dependent libraries of Eigen won't compile. To solve the problem set the following environment variable before running cmake:

```powershell
$env:CMAKE_POLICY_VERSION_MINIMUM = "3.5"
```

Note that this is a temporary setting and is valid only for the current PowerShell session.

## Requirements

You need to install and run an instance of the Rerun viewer. You can download it from [here](https://rerun.io/docs/getting-started/installing-viewer#installing-the-viewer).

We suggest to install it via pip:

```bash
python -m venv .venv
source .venv/bin/activate  # On Windows use `.venv\Scripts\activate
pip install rerun-sdk==0.35.0
````

Note that the SDK version that you specify in the pip installation must match the version of the Rerun SDK used to build the plugin (see `CMakeLists.txt` in the `FetchContent_Declare(rerun_sdk` section). At the moment, some of the rerun-sdk dependencies (arrow library) for rerun-sdk versions newer than 0.27.3 cannot build on cmake 4.x.


## Concepts

The plugin plots on Rerun the data received from different topics accoring to a list of *keypaths*. Each keypath is a dot-separated string that specifies the path to a particular value in the input JSON data. For example, if the input JSON data is: 

```json
{
  "sensor": {
    "temperature": 22.5,
    "humidity": 60
  }
}
```

the keypaths to access the temperature and humidity values would be `/sensor/temperature` and `/sensor/humidity`, respectively.

Note that to each keypath is prepended the topic name, so if the topic is `replay`, the full keypaths would be `/replay/sensor/temperature` and `/replay/sensor/humidity`. Notice the **mandatory leading slash**: only (and all) keypaths starting with `/` are parsed according to this syntax.

The plugin uses the `timecode` field in the input data to determine the time at which each data point was recorded. If the `timecode` field is not present, the plugin uses the elapsed time since the start of the plugin as the time for each data point. If you want to use a different field name for the timecode, you can specify it in the INI settings (see below).

At the moment, the plugin also supports the calculation of the autocorrelation function (ACF) and the FFT spectrum for specified keypaths. The ACF is a measure of how the values of a time series are correlated with themselves at different time lags. This can be useful for identifying patterns and periodicities in the data, as well to identify the minimum timestep (or maximum frequency) to sample the data. To add the ACF for a keypath, simply include it in the `acf_keypaths` list in the INI settings (see below). To add the FFT, include the `fft_keypaths` list. The width of the ACF and FFT windows can be specified using the `window_size` setting.

An alternative keypaths syntax uses dots as separators. For example, `/IMU/accel/0/x` can also be written as `IMU.accel[0].x`. This syntax is for backward portability, but the slash syntax is to be preferred, since it maps to a proper hierarchy on Rerun.

Note that the ACF bar chart has a **lag number** on abscissa, i.e. each bar represents a single lag. The FFT bar chart, conversely, has a frequency step on the abscissa, so that if the window has 50 samples, the FFT has 25 bars spanning the frequencies from 0 to 1/(2*DT), being DT the time duration of the window. The next release for Rerun (after v0.24.1) will allow to explicitly set the abscissa scale on bar charts.

With Rerun versions supporting the abscissa (method `rerun::BarChart::with_abscissa()`), you can enable it with:

```bash
cmake -Bbuild -DWITH_ABSCISSA:BOOL=FALSE
```

## How Rerun gets the data

The Rerun viewer receives the data on a path that depends on the type:

* Numeric values: under the root path `data/`
* Boolean values: under the root path `flags/`
* Text values: under the root path `states/`

## Blueprints

Rerun viewer supports **blueprints**: those are files describing the layout of panels and controls in the viewer. You can configure the layout to your liking, then save the corresponding blueprint (from the menu with the Rerun logo on the window top left corner, select *save blueprint...*). Then add a `blueprint` key to the INI file (see below) pointing to the `.rbl` file.


## Rerun console output

The Rerun SDK prints logging messages on the console. If you don't want it to pullute the plugin output, you can disable it:

```bash
export RUST_LOG=error rerun
```


## INI settings

The plugin supports the following settings in the INI file:

```ini
[rerunner]
sub_topic = ["replay"] # Topics to subscribe to (array of strings)
time = "timecode"       # Name of the timecode column in the input data (string)
# List of keypaths for time series
keypaths = ["/replay/sensor/temperature", "/replay/sensor/humidity"]
# List of keypaths to build ACF for
acf_keypaths = ["/replay/sensor/temperature", "/replay/sensor/humidity"]
fft_keypaths = ["/replay/sensor/temperature", "/replay/sensor/humidity"]
window_size  = 200          # Width of the ACF and FFT window (integer)
blueprint = "blueprint.rbl" # local or absolute path to a valid blueprint
parallelize = true # Runs FFT and ACF computations in parallel
```

All settings are optional; if omitted, the default values are used.


## Example data

The file `example.csv` contains example data that can be used to test the plugin. You can replay this data using the `replay_plugin` plugin in MADS (see <https://github.com/MADS-NET/replay_plugin>), and visualize it using the `rerun_play` plugin.

---
## 3D reference frames

Reference-frame visualization is ported from
[alfatave/rerunner_plugin](https://github.com/alfatave/rerunner_plugin/commit/33f4660)
onto the current official plugin (MADS v2.5-P8, Rerun SDK 0.35.0).

Configure matrix keypaths and the displayed axis length:

```toml
[rerunner]
sub_topic = ["robot"]
hmat_keypaths = ["/robot/pose"]
h_scale = 0.2
```

For example, send this JSON on topic `robot`:

```json
{
  "timecode": 1.0,
  "pose": [
    [0, -1, 0, 1],
    [1,  0, 0, 2],
    [0,  0, 1, 3],
    [0,  0, 0, 1]
  ]
}
```

This draws a frame at `(1, 2, 3)` rotated 90 degrees about Z, with
red X, green Y, and blue Z arrows labeled accordingly. Add a 3D view in
Rerun and include the `frame/` subtree (the example appears at
`frame/robot/pose`). Frames use the same `time` timeline as other data.

Matrices can also be flat arrays of 16 numbers in **row-major** order.
The first three columns give the axes; the fourth column gives the origin.
The last row must be `[0, 0, 0, 1]` (tolerance `1e-6`). All entries and
resulting coordinates must be finite and representable by the viewer.
The rotation block is used as supplied, without orthonormalization.

`h_scale` defaults to `1.0` and must be finite and positive. It scales only
the arrows, not the translation; use consistent units for all 3D entities.
The skeleton's `3D_scale` setting does not affect reference frames.
`hmat_keypaths` defaults to an empty list, so the feature is opt-in.
Legacy dot paths such as `robot.pose` and slash-containing topics such as
`robot/arm` (keypath `/robot/arm/pose`) are supported. Messages from other
topics and missing matrix fields are skipped. Present but malformed matrices
return a plugin error identifying the keypath.

To run the geometry and plugin integration tests without launching a viewer:

```sh
cmake -S . -B build -DRERUNNER_BUILD_TESTS=ON
cmake --build build --target reference_frame_test reference_frame_plugin_test
ctest --test-dir build --output-on-failure
```
