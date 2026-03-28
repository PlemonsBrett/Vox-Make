"""Tests for test audio generator."""

from __future__ import annotations

import numpy as np
import pytest

from tools.audio_gen import generate_chirp, generate_sine, generate_white_noise


class TestGenerateSine:
    """Sine tone generator tests."""

    @pytest.mark.unit
    def test_correct_sample_count(self) -> None:
        samples = generate_sine(440.0, 2.0, sample_rate=16000)
        assert len(samples) == 32000

    @pytest.mark.unit
    def test_output_is_int16(self) -> None:
        samples = generate_sine(440.0, 1.0)
        assert samples.dtype == np.int16

    @pytest.mark.unit
    def test_peak_amplitude_does_not_clip(self) -> None:
        samples = generate_sine(440.0, 1.0, amplitude=0.8)
        assert np.max(np.abs(samples)) <= 32767

    @pytest.mark.unit
    def test_spectral_peak_at_target_frequency(self) -> None:
        """FFT of sine should peak at the target frequency."""
        sample_rate = 16000
        freq = 440.0
        samples = generate_sine(freq, 1.0, sample_rate=sample_rate, amplitude=0.8)

        spectrum = np.abs(np.fft.rfft(samples.astype(np.float64)))
        freqs = np.fft.rfftfreq(len(samples), 1.0 / sample_rate)
        peak_freq = freqs[np.argmax(spectrum)]

        assert abs(peak_freq - freq) < sample_rate / len(samples)  # within 1 bin


class TestGenerateWhiteNoise:
    """White noise generator tests."""

    @pytest.mark.unit
    def test_correct_sample_count(self) -> None:
        samples = generate_white_noise(3.0, sample_rate=16000)
        assert len(samples) == 48000

    @pytest.mark.unit
    def test_deterministic_with_seed(self) -> None:
        a = generate_white_noise(1.0, seed=123)
        b = generate_white_noise(1.0, seed=123)
        np.testing.assert_array_equal(a, b)

    @pytest.mark.unit
    def test_different_seeds_produce_different_output(self) -> None:
        a = generate_white_noise(1.0, seed=1)
        b = generate_white_noise(1.0, seed=2)
        assert not np.array_equal(a, b)


class TestGenerateChirp:
    """Chirp (frequency sweep) generator tests."""

    @pytest.mark.unit
    def test_correct_sample_count(self) -> None:
        samples = generate_chirp(100.0, 4000.0, 2.0, sample_rate=16000)
        assert len(samples) == 32000

    @pytest.mark.unit
    def test_energy_spans_frequency_range(self) -> None:
        """Chirp spectrum should have energy across the sweep range."""
        sample_rate = 16000
        samples = generate_chirp(200.0, 3000.0, 2.0, sample_rate=sample_rate)

        spectrum = np.abs(np.fft.rfft(samples.astype(np.float64)))
        freqs = np.fft.rfftfreq(len(samples), 1.0 / sample_rate)

        # Energy in sweep band should dominate
        in_band = spectrum[(freqs >= 200) & (freqs <= 3000)]
        out_band = spectrum[(freqs > 3000) & (freqs <= sample_rate / 2)]

        assert np.mean(in_band) > 3 * np.mean(out_band)
