# Architecture Notes

This project is intended to grow around a shared overlay model with
platform-specific hardware adapters.

## Initial Boundaries

- Overlay content model: what is displayed, positioned, updated, and removed.
- Rendering surface: how overlay content reaches an AR/XR display target.
- Hardware adapter: platform-specific integration for Linux, Android, iOS, or
  Windows.
- Assets: calibration inputs, sample overlays, and visual resources used by the
  rendering path.

## Near-Term Decisions

- Choose the first implementation language and build system.
- Define a minimal overlay scene or message model.
- Identify the first AR/XR glasses or host bridge to validate against.
- Add a smoke test that can run without hardware.
