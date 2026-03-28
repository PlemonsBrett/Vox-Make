"""Generate test audio files for C/C++ pipeline validation.

Produces WAV files with known properties so C-side tests can verify
against ground truth without depending on real recordings.

Usage:
    uv run generate-test-audio --type sine --freq 440 --duration 2.0 -o test.wav
    uv run generate-test-audio --type noise --color white --duration 5.0 -o noise.wav
    uv run generate-test-audio --type chirp --freq-start 100 --freq-end 4000 -o chirp.wav
"""

from __future__ import annotations

from pathlib import Path
from typing import TYPE_CHECKING

import click
import numpy as np
from scipy.io import wavfile

if TYPE_CHECKING:
    from numpy.typing import NDArray


def generate_sine(
    freq_hz: float,
    duration_s: float,
    sample_rate: int = 16000,
    amplitude: float = 0.8,
) -> NDArray[np.int16]:
    """Generate a pure sine tone as 16-bit PCM samples."""
    t = np.arange(int(sample_rate * duration_s)) / sample_rate
    samples = amplitude * np.sin(2.0 * np.pi * freq_hz * t)
    return (samples * 32767).astype(np.int16)


def generate_white_noise(
    duration_s: float,
    sample_rate: int = 16000,
    amplitude: float = 0.3,
    seed: int = 42,
) -> NDArray[np.int16]:
    """Generate white noise as 16-bit PCM samples."""
    rng = np.random.default_rng(seed)
    n_samples = int(sample_rate * duration_s)
    samples = amplitude * rng.standard_normal(n_samples)
    return np.clip(samples * 32767, -32768, 32767).astype(np.int16)


def generate_chirp(
    freq_start: float,
    freq_end: float,
    duration_s: float,
    sample_rate: int = 16000,
    amplitude: float = 0.8,
) -> NDArray[np.int16]:
    """Generate a linear chirp (frequency sweep) as 16-bit PCM samples."""
    t = np.arange(int(sample_rate * duration_s)) / sample_rate
    phase = 2.0 * np.pi * (freq_start * t + (freq_end - freq_start) * t**2 / (2.0 * duration_s))
    samples = amplitude * np.sin(phase)
    return (samples * 32767).astype(np.int16)


def write_wav(path: Path, samples: NDArray[np.int16], sample_rate: int = 16000) -> None:
    """Write 16-bit mono PCM WAV file."""

    wavfile.write(str(path), sample_rate, samples)


@click.command("generate-test-audio")
@click.option(
    "--type",
    "signal_type",
    type=click.Choice(["sine", "noise", "chirp"]),
    required=True,
    help="Signal type to generate.",
)
@click.option("--freq", default=440.0, help="Frequency in Hz (sine) or ignored.")
@click.option("--freq-start", default=100.0, help="Start frequency for chirp.")
@click.option("--freq-end", default=4000.0, help="End frequency for chirp.")
@click.option("--duration", default=2.0, help="Duration in seconds.")
@click.option("--sample-rate", default=16000, help="Sample rate in Hz.")
@click.option("-o", "--output", type=click.Path(), required=True, help="Output WAV path.")
def cli(
    signal_type: str,
    freq: float,
    freq_start: float,
    freq_end: float,
    duration: float,
    sample_rate: int,
    output: str,
) -> None:
    """Generate test audio files with known spectral properties."""
    out_path = Path(output)

    if signal_type == "sine":
        samples = generate_sine(freq, duration, sample_rate)
        click.echo(f"Generated {duration}s sine tone at {freq}Hz")
    elif signal_type == "noise":
        samples = generate_white_noise(duration, sample_rate)
        click.echo(f"Generated {duration}s white noise")
    elif signal_type == "chirp":
        samples = generate_chirp(freq_start, freq_end, duration, sample_rate)
        click.echo(f"Generated {duration}s chirp {freq_start}→{freq_end}Hz")
    else:
        msg = f"Unknown signal type: {signal_type}"
        raise click.BadParameter(msg)

    write_wav(out_path, samples, sample_rate)
    click.echo(f"Wrote {out_path} ({len(samples)} samples @ {sample_rate}Hz)")
