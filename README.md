# NQUI

Modern Qt Widgets UI component library and demos, focused on dashboard-style interfaces and frameless desktop windows.

## Highlights

- Modular component architecture: reusable UI widgets as independent libraries
- Frameless window support with smooth rounded corners and resize/move handling
- Dashboard components with dynamic runtime APIs (color, spacing, radius, value, etc.)
- Demo-driven development: each encapsulated component is verified in demo applications
- Native Qt/QMake workflow for Windows desktop development

## Features

### `NFramelessWidget`

- Frameless window base class
- Dynamic corner radius (`setCornerRadius`)
- Shadow styles and configurable resize/move behavior
- Improved rounded-corner anti-aliasing for cleaner edges

### `NDashboard` (plugin library)

- App shell and layout containers
- Header, page title/filter, sidebar navigation
- Notification bell, primary action button, compact profile widget
- Metric cards, trend indicator, circular gauge, sparkline card
- Time-series line chart panel and grouped bar chart panel
- Runtime customization for visual tokens (background, accent, radius, spacing)

### Demos / App

- `Demo1`: base frameless window demo
- `DemoDashboard`: dashboard component gallery and interaction demo
- `NApp`: integrated application-level example combining `NFramelessWidget` + `NDashboard`

## Project Structure

```text
NQUI/
  NQUI.pro
  NPlugins/
    NFramelessWidget/
    NDashboard/
  Demos/
    Demo1/
    DemoDashboard/
  NApp/
```

## Requirements

- Qt 5.14+ (Widgets + Charts)
- C++11 compatible compiler
- QMake build system
- Windows + MSVC (recommended for current setup)

## Build

### 1) Configure environment

- Open a developer shell with MSVC environment loaded (`vcvars*.bat`)
- Ensure `qmake` and `jom` (or `nmake`) are available in `PATH`

### 2) Build all subprojects

```powershell
cd NQUI
qmake NQUI.pro
jom
```

> `NQUI.pro` uses ordered subdirs build to satisfy plugin/demo link dependencies.

### 3) Build a single target (example)

```powershell
cd NQUI/NApp
qmake NApp.pro
jom
```

## Run

- Launch generated binaries from `build/bin/<arch>/<config>/`
- Typical entry targets:
  - `Demo1.exe`
  - `DemoDashboard.exe`
  - `NApp.exe`

## Development Notes

- Component APIs are designed for runtime theme/property adjustment
- Demo pages are the primary validation path for visual correctness
- For UI changes, verify both behavior and visual spacing/alignment

## Roadmap

- More theme tokens and preset styles
- More business dashboard widgets (tables, filters, KPI layouts)
- Better cross-platform packaging workflow
- Unit/integration tests for reusable components

## Contributing

Contributions are welcome.

1. Fork the repository
2. Create a feature branch
3. Make focused commits with clear messages
4. Open a Pull Request with screenshots / demo notes for UI changes

## License

No license file is currently provided in the repository.

If you plan to open-source this project publicly, add a license file (for example, MIT/Apache-2.0) before external distribution.
