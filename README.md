# ShimmerVerb for KORG NTS-1

This is a custom `revfx` unit for the KORG NTS-1 digital kit / logue SDK.

## Controls

- `TIME`: reverb decay
- `DEPTH`: wet mix
- `SHIFT + DEPTH`: shimmer amount, feeding an octave-up tail back into the reverb

## Build

Install the official KORG logue SDK first, including its submodules and Arm toolchain.

From this folder:

```sh
make PLATFORMDIR=/path/to/logue-sdk/platform/nutekt-digital install
```

The output will be:

```text
shimmer_revfx.ntkdigunit
```

Load that file with NTS-1 Sound Librarian.

You can also copy this whole `shimmer-revfx` folder into:

```text
logue-sdk/platform/nutekt-digital/
```

Then build from `logue-sdk/platform/nutekt-digital/shimmer-revfx` with:

```sh
make install
```

## Notes

The algorithm is a compact shimmer reverb:

- 4-line feedback delay network for the reverb body
- input diffusion through two allpass stages
- dual-tap moving-delay octave-up shifter
- high-passed shimmer feedback to keep the low end stable

If the tail blooms too aggressively on the real unit, lower `SHIFT + DEPTH` first.

KORG template files are covered by `LICENSE-KORG-BSD-3-Clause.txt`.
