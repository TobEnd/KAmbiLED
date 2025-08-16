# Hyperion Processor Algorithm

This document outlines the algorithm implemented in the `HyperionProcessor` module, which aims to replicate the core image processing pipeline of Hyperion.ng for generating LED colors from screen content.

## Overall Goal

The primary goal of the `HyperionProcessor` is to transform a captured screen image into a set of colors suitable for driving an LED strip, creating an Ambilight-like effect. It focuses on accurately sampling colors from relevant areas of the screen while ignoring irrelevant parts like black borders.

### Configurable Parameters and Defaults

The behavior of the `HyperionProcessor` can be influenced by several parameters, which are passed during its initialization via a `QJsonObject`. These parameters allow for fine-tuning the algorithm's response to different content and user preferences.

*   **`blackBorderThreshold`**: This parameter controls the sensitivity of the black border detection. It is a floating-point value.
    *   **Default Value:** `0.1`
*   **`smoothingFactor`**: This parameter determines the degree of color smoothing applied to the calculated LED colors. It is a floating-point value, where higher values result in smoother transitions between colors.
    *   **Default Value:** `0.1`

Note that the LED layout (number of LEDs on each side) is provided separately via the `LedLayout` struct and is not part of this `QJsonObject` configuration.

## Processing Steps

The `HyperionProcessor` follows a multi-step pipeline to achieve this:

### 1. Black Border Detection

Before sampling colors, the processor identifies and accounts for black borders (letterboxing or pillarboxing) in the input image. This ensures that the LEDs only react to the actual video content and not to the black bars.

*   **Component:** `BlackBorderDetector`
*   **Mechanism:** The `BlackBorderDetector` analyzes the edges of the input image. It samples pixels at strategic locations (e.g., 1/3, 2/3, and center of the image dimensions) to find the first non-black pixel. This helps determine the extent of the black borders.
*   **Output:** A `BlackBorder` struct containing the `horizontalSize` and `verticalSize` of the detected borders. If no borders are detected, these sizes will be zero.

### 2. LED Area Mapping (`buildLedMap`)

This is a crucial step where the relationship between the LEDs and specific regions of the screen content is established. Unlike simple pixel sampling, Hyperion maps each LED to a collection of pixels within a defined area.

*   **Component:** `HyperionProcessor::buildLedMap`
*   **Mechanism:**
    1.  It first calculates the dimensions of the *content area* by subtracting the detected black border sizes from the overall image dimensions.
    2.  For each LED defined in the `LedLayout` (e.g., bottom, right, top, left LEDs), it determines a corresponding rectangular region within this content area.
    3.  It then iterates through all the pixels within that calculated region and stores their coordinates (x, y) in an internal `_ledMap` (a `QVector<QVector<QPoint>>`). Each inner `QVector<QPoint>` represents the set of pixels associated with a single LED.
*   **Rebuilding the Map:** The `_ledMap` is rebuilt only when the input image dimensions change or when the detected black borders change. This optimizes performance by avoiding unnecessary recalculations.

### 3. Color Calculation (`calculateLedColors`)

Once the `_ledMap` is established, the processor calculates the final color for each LED based on the pixels mapped to it.

*   **Component:** `HyperionProcessor::calculateLedColors` and `HyperionProcessor::getAverageColor`
*   **Mechanism:**
    1.  It iterates through each LED's entry in the `_ledMap`.
    2.  For each LED, it retrieves all the associated pixel coordinates.
    3.  It then calls `getAverageColor` to compute a single representative color for that LED.
*   **Current Color Algorithm (Mean Color):** Currently, the `getAverageColor` function calculates the simple arithmetic mean of the Red, Green, and Blue components of all pixels within an LED's mapped region. This provides a smooth and representative color for the area.
    *   *Future Enhancements:* Hyperion.ng offers other color calculation methods (e.g., mean squared, dominant color, k-means clustering) which could be implemented here for different visual effects.

## What We Are Trying to Achieve Here (with `grabberconfigurator`)

The `grabberconfigurator` tool serves as a visual debugging and testing utility for this `HyperionProcessor` pipeline. Its purpose is to:

1.  **Verify Screen Capture and Scaling:** Confirm that the `WaylandGrabber` and `HyperionGrabber` are correctly capturing and scaling the screen content.
2.  **Validate Black Border Detection:** Visually inspect if the `BlackBorderDetector` is accurately identifying and cropping the black borders from the input image. The current visualization in `grabberconfigurator` shows the image *after* the black borders have been removed.
3.  **Test LED Mapping:** Observe how the `_ledMap` is generated and how the LED regions are defined on the screen content. The visualization outlines these regions.
4.  **Evaluate Color Calculation:** Ultimately, this tool will allow us to see the final calculated LED colors in real-time, helping us fine-tune the color calculation algorithms and LED layout for the best Ambilight experience.

By breaking down the complex process into these observable steps, we can systematically develop and refine our Hyperion-like image processing capabilities.

## Original Hyperion.ng Default Parameters

This section outlines the default configuration parameters used by the original Hyperion.ng project for its image processing and related functionalities, as defined in its schema files. These values represent the out-of-the-box behavior of Hyperion.ng when no custom configuration is provided.

### Black Border Detector Defaults (`schema-blackborderdetector.json`)

*   **`enable`**: `true` - Whether black border detection is enabled by default.
*   **`threshold`**: `5` (percentage) - The luminance threshold for detecting black borders. Pixels below this threshold are considered black.
*   **`unknownFrameCnt`**: `600` - Number of frames to wait before re-evaluating black borders if the state is uncertain.
*   **`borderFrameCnt`**: `50` - Number of consecutive frames required to confirm a stable black border state.
*   **`maxInconsistentCnt`**: `10` - Maximum number of inconsistent frames allowed before re-evaluating black borders.
*   **`blurRemoveCnt`**: `1` - Number of pixels to blur at the edges to remove artifacts before black border detection.
*   **`mode`**: `"default"` - The black border detection mode. Other options include "classic", "osd", "letterbox".

### Smoothing Defaults (`schema-smoothing.json`)

*   **`enable`**: `true` - Whether color smoothing is enabled by default.
*   **`type`**: `"linear"` - The type of smoothing algorithm used. Other options include "decay".
*   **`time_ms`**: `150` (milliseconds) - The time over which colors are smoothed.
*   **`updateFrequency`**: `25.0` (Hz) - The frequency at which the smoothed colors are updated.
*   **`interpolationRate`**: `1.0` (Hz) - The rate at which interpolation is performed (relevant for "decay" smoothing).
*   **`decay`**: `1.0` - The decay factor for "decay" smoothing.
*   **`dithering`**: `true` - Whether dithering is applied during smoothing.
*   **`updateDelay`**: `0` (frames) - Number of frames to delay the update of smoothed colors.

