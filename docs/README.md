# KAmbi

KAmbiLED is a Hyperion image grabber for Wayland / Linux, designed to capture screen content, process it using a Hyperion-like algorithm, and send the calculated LED colors directly to a WLED device. This project was inspired by the original [Hyperion](https://hyperion-project.org/) project.

## Features

*   **Wayland Screen Capture:** Efficiently captures screen content on Wayland.
*   **Hyperion-like Processing:** Applies color mapping and smoothing algorithms to generate LED colors.
*   **WLED Integration:** Sends processed LED colors to WLED devices via UDP Realtime protocol.
*   **Flexible Configuration:** Supports configuration via command-line arguments and `.env` files, with command-line arguments taking precedence.

## Installation

### Dependencies

Before compiling, you need to install the required dependencies for your Linux distribution.

**Arch Linux:**

```bash
sudo pacman -S qt6-base qt6-multimedia
```

**Ubuntu/Debian:**

```bash
sudo apt-get update
sudo apt-get install qt6-base-dev qt6-multimedia-dev
```

**Fedora:**

```bash
sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel
```

### Compilation

To compile, navigate to the project root and run:

```bash
cmake . && make
```

## Usage

After successful compilation, you can run KAmbiLED from the `build` directory.

```bash
cd build
./KAmbiLED
```

KAmbiLED can be configured using command-line arguments or by providing a `.env` file in the application's directory. Command-line arguments will always override values set in the `.env` file.

### Configuration Parameters

Here's a list of available configuration parameters:

*   **`-d`, `--debug`**: Enable verbose debug output.
*   **`--wled-address <address>`**: IP address or hostname of your WLED device (e.g., `192.168.1.177`).
    *   *Environment Variable:* `KAMBILED_WLED_ADDRESS`
*   **`--wled-port <port>`**: UDP port of your WLED device for Realtime data (default: `21324`).
    *   *Environment Variable:* `KAMBILED_WLED_PORT`
*   **`--wled-color-order <order>`**: Color order of your WLED device (e.g., `RGB`, `GRB`, `BGR`) (default: `GRB`).
    *   *Environment Variable:* `KAMBILED_WLED_COLOR_ORDER`
*   **`--scale <factor>`**: Divisor used to scale your screen resolution (e.g., `8`). If your screen is `1920x1080` and `SCALE` is `8`, the image sent to HyperionProcessor is `240x135`. A smaller value means higher resolution and more CPU/bandwidth usage. (default: `8`)
    *   *Environment Variable:* `KAMBILED_SCALE`
*   **`--frameskip <count>`**: Number of frames to skip between captures (`0` means no frames are skipped). (default: `0`)
    *   *Environment Variable:* `KAMBILED_FRAMESKIP`
*   **`--change-threshold <threshold>`**: Threshold for image change detection (`0-255`). Only emit new frames if difference exceeds this. Higher values reduce flickering but might miss subtle changes. (default: `100`)
    *   *Environment Variable:* `KAMBILED_CHANGE_THRESHOLD`
*   **`--leds-bottom <count>`**: Number of LEDs on the bottom edge of your display (e.g., `70`).
    *   *Environment Variable:* `KAMBILED_LEDS_BOTTOM`
*   **`--leds-right <count>`**: Number of LEDs on the right edge of your display (e.g., `20`).
    *   *Environment Variable:* `KAMBILED_LEDS_RIGHT`
*   **`--leds-top <count>`**: Number of LEDs on the top edge of your display (e.g., `70`).
    *   *Environment Variable:* `KAMBILED_LEDS_TOP`
*   **`--leds-left <count>`**: Number of LEDs on the left edge of your display (e.g., `20`).
    *   *Environment Variable:* `KAMBILED_LEDS_LEFT`
*   **`-o`, `--offset <offset>`**: Shift the starting position of the LEDs (e.g., `10`) (default: `0`).
    *   *Environment Variable:* `KAMBILED_LED_OFFSET`
*   **`--clockwise <true/false>`**: Direction of the LED strip (`true` for clockwise, `false` for counter-clockwise) (default: `false`).
    *   *Environment Variable:* `KAMBILED_LED_CLOCKWISE`
*   **`--bb-enable <true/false>`**: Enable black border detector. (default: `true`)
    *   *Environment Variable:* `KAMBILED_BB_ENABLE`
*   **`--bb-threshold <threshold>`**: Black border detector threshold. (default: `5`)
    *   *Environment Variable:* `KAMBILED_BB_THRESHOLD`
*   **`--bb-unknown-frame-cnt <count>`**: Black border detector unknown frame count. (default: `600`)
    *   *Environment Variable:* `KAMBILED_BB_UNKNOWN_FRAME_CNT`
*   **`--bb-border-frame-cnt <count>`**: Black border detector border frame count. (default: `50`)
    *   *Environment Variable:* `KAMBILED_BB_BORDER_FRAME_CNT`
*   **`--bb-max-inconsistent-cnt <count>`**: Black border detector max inconsistent count. (default: `10`)
    *   *Environment Variable:* `KAMBILED_BB_MAX_INCONSISTENT_CNT`
*   **`--bb-blur-remove-cnt <count>`**: Black border detector blur remove count. (default: `1`)
    *   *Environment Variable:* `KAMBILED_BB_BLUR_REMOVE_CNT`
*   **`--bb-mode <mode>`**: Black border detector mode. (default: `default`)
    *   *Environment Variable:* `KAMBILED_BB_MODE`
*   **`--smooth-enable <true/false>`**: Enable smoothing. (default: `true`)
    *   *Environment Variable:* `KAMBILED_SMOOTH_ENABLE`
*   **`--smooth-type <type>`**: Smoothing type. (default: `linear`)
    *   *Environment Variable:* `KAMBILED_SMOOTH_TYPE`
*   **`--smooth-time-ms <ms>`**: Smoothing time in milliseconds. (default: `150`)
    *   *Environment Variable:* `KAMBILED_SMOOTH_TIME_MS`
*   **`--smooth-update-frequency <frequency>`**: Smoothing update frequency. (default: `25.0`)
    *   *Environment Variable:* `KAMBILED_SMOOTH_UPDATE_FREQUENCY`
*   **`--smooth-interpolation-rate <rate>`**: Smoothing interpolation rate. (default: `1.0`)
    *   *Environment Variable:* `KAMBILED_SMOOTH_INTERPOLATION_RATE`
*   **`--smooth-decay <decay>`**: Smoothing decay. (default: `1.0`)
    *   *Environment Variable:* `KAMBILED_SMOOTH_DECAY`
*   **`--smooth-dithering <true/false>`**: Enable smoothing dithering. (default: `true`)
    *   *Environment Variable:* `KAMBILED_SMOOTH_DITHERING`
*   **`--smooth-update-delay <delay>`**: Smoothing update delay. (default: `0`)
    *   *Environment Variable:* `KAMBILED_SMOOTH_UPDATE_DELAY`
*   **`--color-algorithm <algorithm>`**: Color algorithm. (default: `mean_sqrt`)
    *   *Environment Variable:* `KAMBILED_COLOR_ALGORITHM`

### Example Usage

To run KAmbiLED with a specific WLED address and LED offset:

```bash
./KAmbiLED --wled-address 192.168.1.177 --offset 10
```

Alternatively, you can create a `.env` file in the same directory as the executable:

```
KAMBILED_WLED_ADDRESS="192.168.1.177"
KAMBILED_LED_OFFSET="10"
```

And then run the executable:

```bash
./KAmbiLED
```

For more detailed information on the Hyperion processing algorithm, please refer to `docs/hyperion_processor_algorithm.md`.

## Contributing

There are probably bugs, please submit a pull request if you can.
