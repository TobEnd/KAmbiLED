# Hyperion_Grabber_Wayland_QT
Hyperion image grabber for Wayland / Linux. This application captures screen content, processes it using a Hyperion-like algorithm, and sends the calculated LED colors directly to a WLED device.

## Compilation

To compile run `cmake . && make`, qt6 (core / gui / multimedia / multimediawidgets ; qt6-base, qt6-multimedia on arch) libraries are required.

## Wayland Proof of Concept

This project includes a proof-of-concept Wayland grabber, `wayland_poc`. This executable is used to test and develop screen capturing on Wayland.

To build and run the Wayland POC:
1.  Make sure you have the required dependencies installed (Qt6, PipeWire).
2.  Build the project: `cmake . && make`
3.  Run the executable: `./wayland_poc`

This will open a window and, after you grant permission in the system's dialog, it will display a capture of your screen.

## Usage

This application is configured entirely through environment variables. This provides flexibility for deployment and integration into various systems.

To run the grabber, set the following environment variables and then execute the `Hyperion_Grabber_Wayland_QT` executable:

```bash
# --- Required WLED Connection Settings ---
# IP address of your WLED device (e.g., 192.168.1.177)
export HYPERION_GRABBER_WLED_ADDRESS="<YOUR_WLED_IP_ADDRESS>"
# UDP port of your WLED device for Realtime data (default: 21324)
export HYPERION_GRABBER_WLED_PORT="21324"
# Color order of your WLED device (e.g., RGB, GRB, BGR) (default: GRB)
export HYPERION_GRABBER_WLED_COLOR_ORDER="GRB"

# --- Grabber Settings ---
# Divisor used to scale your screen resolution (e.g., 8)
# If your screen is 1920x1080 and SCALE is 8, the image sent to HyperionProcessor is 240x135.
# A smaller value means higher resolution and more CPU/bandwidth usage.
export HYPERION_GRABBER_SCALE="8"
# Number of frames to skip between captures (0 means no frames are skipped)
export HYPERION_GRABBER_FRAMESKIP="0"
# Threshold for image change detection (0-255). Only emit new frames if difference exceeds this.
# Higher values reduce flickering but might miss subtle changes. (default: 100)
export HYPERION_GRABBER_CHANGE_THRESHOLD="100"

# --- LED Layout Configuration ---
# Number of LEDs on the bottom edge of your display (e.g., 70)
export HYPERION_GRABBER_LEDS_BOTTOM="70"
# Number of LEDs on the right edge of your display (e.g., 20)
export HYPERION_GRABBER_LEDS_RIGHT="20"
# Number of LEDs on the top edge of your display (e.g., 70)
export HYPERION_GRABBER_LEDS_TOP="70"
# Number of LEDs on the left edge of your display (e.g., 20)
export HYPERION_GRABBER_LEDS_LEFT="20"
# Shift the starting position of the LEDs (e.g., 10) (default: 0)
export HYPERION_GRABBER_LED_OFFSET="0"
# Direction of the LED strip (e.g., true for clockwise, false for counter-clockwise) (default: false)
export HYPERION_GRABBER_LED_CLOCKWISE="false"

# --- Hyperion Algorithm Settings ---
# Black border detection threshold (0.0 to 1.0). Higher values are more aggressive.
# This helps ignore dark UI elements or black bars in videos. (default: 0.2)
export HYPERION_GRABBER_BLACK_BORDER_THRESHOLD="0.2"
# Smoothing factor for color transitions (0.0 to 1.0). Higher values result in smoother transitions.
# 0.0 = no smoothing, 1.0 = full smoothing (colors won't change). (default: 0.1)
export HYPERION_GRABBER_SMOOTHING_FACTOR="0.1"

# --- Run the application ---
./Hyperion_Grabber_Wayland_QT
```

Replace `<YOUR_WLED_IP_ADDRESS>` with the actual IP address of your WLED device.

For more detailed information on the Hyperion processing algorithm and development lessons learned, please refer to the `docs/` directory.

## Contributing

There are probably bugs, please submit a pull request if you can.