# FastLED Math

This folder contains files extracted and modified from the FastLED library.

For the original source of these files, please refer to: https://github.com/FastLED/FastLED

These files have been modified to allow the use of the FastLED math functions in a desktop environment.

This modification primarily consists of:
- Removing some GCC-specific attributes
- Removing some checks for platform definitions

All functionality has been placed into the `flm` namespace (for FastLED Math).