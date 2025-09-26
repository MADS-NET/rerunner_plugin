# rerunner plugin for MADS

This is a Sink plugin for [MADS](https://github.com/MADS-NET/MADS) that uses [Rerun](https://www.rerun.io/) to visualize time series data.

The plugin allows you to log time series data to Rerun, where it can be visualized as time series. It supports automatic calculation of the autocorrelation function (ACF) for specified keypaths, making it easier to analyze the temporal dependencies in your data.

*Required MADS version: 1.3.5 and rerun-sdk version 0.25.0*


## Supported platforms

Currently, the supported platforms are:

* **Linux** 
* **MacOS**
* **Windows**


## Installation

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

## Requirements

You need to install and run an instance of the Rerun viewer. You can download it from [here](https://rerun.io/docs/getting-started/installing-viewer#installing-the-viewer).


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