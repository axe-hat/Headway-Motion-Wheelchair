/*
 * filters.h — Lightweight signal-processing filters for sensor data
 *
 * Two filters are provided:
 *
 *   MovingAverageFilter  – Sliding-window mean.  Good for removing
 *                          high-frequency jitter while keeping latency low.
 *
 *   LowPassFilter        – Exponential smoothing (first-order IIR).
 *                          A single "alpha" parameter controls the trade-off
 *                          between responsiveness and smoothness.
 */

#ifndef FILTERS_H
#define FILTERS_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// MovingAverageFilter
// ---------------------------------------------------------------------------

#define MAX_WINDOW_SIZE 20  // Maximum samples the moving-average can hold

class MovingAverageFilter {
public:
    /**
     * @param windowSize Number of samples to average (clamped to MAX_WINDOW_SIZE)
     */
    MovingAverageFilter(uint8_t windowSize);

    /**
     * Feed a new sample into the filter.
     * @param value Raw sensor reading
     * @return Filtered (averaged) value
     */
    float update(float value);

    /** Reset the buffer and running sum. */
    void reset();

private:
    float   _buffer[MAX_WINDOW_SIZE];
    float   _sum;
    uint8_t _windowSize;
    uint8_t _index;
    uint8_t _count;
};

// ---------------------------------------------------------------------------
// LowPassFilter
// ---------------------------------------------------------------------------

class LowPassFilter {
public:
    /**
     * @param alpha Smoothing factor in (0, 1].
     *              Small alpha = heavier smoothing, slower response.
     *              alpha = 1.0  = no filtering (pass-through).
     */
    LowPassFilter(float alpha);

    /**
     * Feed a new sample into the filter.
     * @param value Raw sensor reading
     * @return Filtered value
     */
    float update(float value);

    /** Reset the filter state. */
    void reset();

private:
    float _alpha;
    float _output;
    bool  _initialised;
};

#endif // FILTERS_H
