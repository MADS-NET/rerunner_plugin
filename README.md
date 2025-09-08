# rerun plugin for MADS

This is a Sink plugin for [MADS](https://github.com/MADS-NET/MADS). 

<provide here some introductory info>

*Required MADS version: 1.3.5.*


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


## INI settings

The plugin supports the following settings in the INI file:

```ini
[rerun]
# Describe the settings available to the plugin
```

All settings are optional; if omitted, the default values are used.


## Datastore
The plugin supports a datastore file `rerun.json` created in a temporary directory for persisting data between runs. Look at the `Datastore` class for more information on how to use it.



## Executable demo

<Explain what happens if the test executable is run>

---