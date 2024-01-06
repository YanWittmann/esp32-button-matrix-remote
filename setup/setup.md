## CLion Setup

### Platform IO

Using https://blog.jetbrains.com/clion/2020/08/arduino-from-hobby-to-prof-p1/ as a guide, we will install the following
software:

- Platform IO
- Platform IO CLion Plugin

Essentially, get Platform IO from: https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html

```shell
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
```

Add whatever path it tells you it installed to your PATH, e.g. `C:\Users\%USERNAME%\.platformio\penv\Scripts`.

Then install the plugin for CLion:

- https://www.jetbrains.com/help/clion/platformio.html
- https://plugins.jetbrains.com/plugin/13922-platformio-for-clion

### Setup project

This is how I set up the project, you do not need to do the same if you already cloned the project. I selected the board
I own, you might have to adjust the [platformio.ini](../platformio.ini) file with your configuration.

![board selection](board-selection.png)

![platformio sidebar](platformio-ide-sidebar.png)

![platformio home project page](platformio-home-project-page.png)

### Add a library

Just for me, how to add a library. All libraries have already been added to the ini file.

https://docs.platformio.org/en/latest/core/userguide/pkg/cmd_install.html#cmd-pkg-install

![platformio home library search](platformio-home-library-search.png)

![platformio home library add.png](platformio-home-library-add.png)

Will result in:

```ini
[env:esp12e]
platform = espressif8266
board = esp12e
framework = arduino
lib_deps = knolleary/PubSubClient@^2.8
```

### Add a library that is unknown to the package manager

You can simply list a git repository or file (see syntax from link in chapter above) in your libraries and it will clone
it into the lib folder.

```ini
lib_deps =
# https://github.com/knolleary/pubsubclient
    knolleary/PubSubClient@^2.8
# https://github.com/ennui2342/arduino-sram
    git+https://github.com/ennui2342/arduino-sram
```

![platformio include non managed library](platformio-include-non-managed-library.png)

Removing dependencies from the platformio.ini file will usually remove the library from the lib folder.

## Keypad

![keypad-board-pins.png](keypad-board-pins.png)

![keypad-board-pins-example.png](keypad-board-pins-example.png)

Code responsible for setting up the Keypad instance.

```c++
constexpr byte ROWS = 4;
constexpr byte COLS = 4;

char keys[ROWS][COLS] = {
    {'1', '2', '3', '4'},
    {'5', '6', '7', '8'},
    {'9', 'A', 'B', 'C'},
    {'D', 'E', 'F', 'G'}
};

byte rowPins[ROWS] = {16, 14, 12, 13}; // 4 3 2 1
byte colPins[COLS] = {5, 4, 0, 2}; // 5 6 7 8

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
```

## Upload the project

For this project in particular:

```shell
pio run -t upload -t monitor
pio run -t upload -t monitor -e esp12e
```

The specification of the environment seems optional, if you only have one environment. The platformio.ini file contains
the `[env:esp12e]` environment.
