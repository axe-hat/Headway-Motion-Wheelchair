"""
test_filters.py — Unit tests for the filter algorithms

Python re-implementations of MovingAverageFilter and LowPassFilter from
filters.h / filters.cpp, with test cases covering:
  - Step response (sudden value change)
  - Constant input (output should converge to that constant)
  - Noise filtering (noisy signal should be smoothed towards the mean)
  - Reset behaviour

Run:  python -m pytest tests/test_filters.py -v
"""

import math
import random
import unittest


# ===========================================================================
# Python re-implementations of the C++ filter classes
# ===========================================================================

class MovingAverageFilter:
    """Sliding-window mean filter (mirrors the C++ version)."""

    MAX_WINDOW_SIZE = 20

    def __init__(self, window_size: int):
        self.window_size = min(window_size, self.MAX_WINDOW_SIZE)
        self.reset()

    def update(self, value: float) -> float:
        self._sum -= self._buffer[self._index]
        self._buffer[self._index] = value
        self._sum += value
        self._index = (self._index + 1) % self.window_size
        if self._count < self.window_size:
            self._count += 1
        return self._sum / self._count

    def reset(self):
        self._buffer = [0.0] * self.MAX_WINDOW_SIZE
        self._sum = 0.0
        self._index = 0
        self._count = 0


class LowPassFilter:
    """First-order IIR / exponential smoothing filter (mirrors the C++ version)."""

    def __init__(self, alpha: float):
        if alpha <= 0.0:
            self._alpha = 0.01
        elif alpha > 1.0:
            self._alpha = 1.0
        else:
            self._alpha = alpha
        self.reset()

    def update(self, value: float) -> float:
        if not self._initialised:
            self._output = value
            self._initialised = True
        else:
            self._output = self._alpha * value + (1.0 - self._alpha) * self._output
        return self._output

    def reset(self):
        self._output = 0.0
        self._initialised = False


# ===========================================================================
# Test cases
# ===========================================================================

class TestMovingAverageFilter(unittest.TestCase):
    """Tests for MovingAverageFilter."""

    def test_constant_input(self):
        """Feeding a constant value should always return that value."""
        f = MovingAverageFilter(5)
        for _ in range(20):
            result = f.update(7.0)
        self.assertAlmostEqual(result, 7.0)

    def test_step_response(self):
        """After filling the window with a new value, output should equal it."""
        window = 5
        f = MovingAverageFilter(window)

        # Fill with 0.0
        for _ in range(window):
            f.update(0.0)

        # Step to 10.0 — output should ramp up over `window` samples
        results = []
        for _ in range(window):
            results.append(f.update(10.0))

        # After exactly `window` samples of 10.0, average should be 10.0
        self.assertAlmostEqual(results[-1], 10.0)
        # First sample after step: (0+0+0+0+10)/5 = 2.0
        self.assertAlmostEqual(results[0], 2.0)

    def test_noise_filtering(self):
        """A noisy signal around a mean should be smoothed towards that mean."""
        f = MovingAverageFilter(10)
        random.seed(42)
        mean_val = 5.0
        noisy = [mean_val + random.gauss(0, 1) for _ in range(100)]

        results = []
        for v in noisy:
            results.append(f.update(v))

        # The last 10 filtered values should be closer to the mean
        # than the last 10 raw values on average
        raw_dev = sum(abs(v - mean_val) for v in noisy[-10:]) / 10
        filt_dev = sum(abs(v - mean_val) for v in results[-10:]) / 10
        self.assertLess(filt_dev, raw_dev)

    def test_reset(self):
        """After reset, the filter should behave as if freshly constructed."""
        f = MovingAverageFilter(3)
        f.update(100.0)
        f.update(100.0)
        f.reset()
        result = f.update(1.0)
        self.assertAlmostEqual(result, 1.0)

    def test_window_clamp(self):
        """Window size exceeding MAX_WINDOW_SIZE should be clamped."""
        f = MovingAverageFilter(999)
        self.assertEqual(f.window_size, MovingAverageFilter.MAX_WINDOW_SIZE)


class TestLowPassFilter(unittest.TestCase):
    """Tests for LowPassFilter."""

    def test_constant_input(self):
        """Feeding a constant value should converge to that value."""
        f = LowPassFilter(0.3)
        for _ in range(100):
            result = f.update(5.0)
        self.assertAlmostEqual(result, 5.0, places=4)

    def test_passthrough_alpha_one(self):
        """Alpha = 1.0 means no filtering — output equals input immediately."""
        f = LowPassFilter(1.0)
        self.assertAlmostEqual(f.update(3.0), 3.0)
        self.assertAlmostEqual(f.update(7.0), 7.0)
        self.assertAlmostEqual(f.update(-2.0), -2.0)

    def test_step_response(self):
        """After a step change, output should approach the new value gradually."""
        f = LowPassFilter(0.1)

        # Initialise at 0.0
        f.update(0.0)

        # Step to 10.0
        results = [f.update(10.0) for _ in range(50)]

        # Output should increase monotonically
        for i in range(1, len(results)):
            self.assertGreaterEqual(results[i], results[i - 1])

        # After 50 iterations with alpha=0.1, should be within 0.1 of 10.0
        self.assertAlmostEqual(results[-1], 10.0, delta=0.1)

    def test_noise_filtering(self):
        """Low-pass filter should reduce variance of a noisy signal."""
        f = LowPassFilter(0.2)
        random.seed(123)
        mean_val = 3.0
        noisy = [mean_val + random.gauss(0, 2) for _ in range(200)]

        results = []
        for v in noisy:
            results.append(f.update(v))

        # Variance of filtered signal should be less than raw
        raw_var = sum((v - mean_val) ** 2 for v in noisy[-50:]) / 50
        filt_var = sum((v - mean_val) ** 2 for v in results[-50:]) / 50
        self.assertLess(filt_var, raw_var)

    def test_reset(self):
        """After reset, the next update should return the new input directly."""
        f = LowPassFilter(0.5)
        f.update(100.0)
        f.update(100.0)
        f.reset()
        result = f.update(1.0)
        self.assertAlmostEqual(result, 1.0)

    def test_alpha_clamp_low(self):
        """Alpha <= 0 should be clamped to 0.01."""
        f = LowPassFilter(-5.0)
        self.assertAlmostEqual(f._alpha, 0.01)

    def test_alpha_clamp_high(self):
        """Alpha > 1 should be clamped to 1.0."""
        f = LowPassFilter(2.5)
        self.assertAlmostEqual(f._alpha, 1.0)


if __name__ == "__main__":
    unittest.main()
