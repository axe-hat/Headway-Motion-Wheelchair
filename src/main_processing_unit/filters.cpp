/*
 * filters.cpp — Implementations of MovingAverageFilter and LowPassFilter
 */

#include "filters.h"

// ===========================================================================
// MovingAverageFilter
// ===========================================================================

MovingAverageFilter::MovingAverageFilter(uint8_t windowSize) {
    _windowSize = min(windowSize, (uint8_t)MAX_WINDOW_SIZE);
    reset();
}

float MovingAverageFilter::update(float value) {
    // Subtract the oldest sample from the running sum
    _sum -= _buffer[_index];

    // Store the new sample
    _buffer[_index] = value;
    _sum += value;

    // Advance the circular index
    _index = (_index + 1) % _windowSize;

    // Track how many samples have been added (up to windowSize)
    if (_count < _windowSize) {
        _count++;
    }

    return _sum / _count;
}

void MovingAverageFilter::reset() {
    _sum   = 0.0f;
    _index = 0;
    _count = 0;
    for (uint8_t i = 0; i < MAX_WINDOW_SIZE; i++) {
        _buffer[i] = 0.0f;
    }
}

// ===========================================================================
// LowPassFilter
// ===========================================================================

LowPassFilter::LowPassFilter(float alpha) {
    // Clamp alpha to valid range
    if (alpha <= 0.0f) {
        _alpha = 0.01f;
    } else if (alpha > 1.0f) {
        _alpha = 1.0f;
    } else {
        _alpha = alpha;
    }
    reset();
}

float LowPassFilter::update(float value) {
    if (!_initialised) {
        // First sample — no previous output to blend with
        _output = value;
        _initialised = true;
    } else {
        // Exponential smoothing: output = alpha * new + (1 - alpha) * prev
        _output = _alpha * value + (1.0f - _alpha) * _output;
    }
    return _output;
}

void LowPassFilter::reset() {
    _output      = 0.0f;
    _initialised = false;
}
