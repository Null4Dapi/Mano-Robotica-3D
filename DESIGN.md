# CyberHand 3D — Web UI Design System

## 1. Visual Identity & Palette
Dark-mode technical instrument aesthetic inspired by aerospace avionics and CAD workstations:

* **Background Page (`--bg-page`)**: `#0a0b0e` (Deep obsidian)
* **Background Panel (`--bg-panel`)**: `#111317` (Charcoal surface)
* **Background Card (`--bg-card`)**: `#14161c` (Elevated card)
* **Subtle Background (`--bg-subtle`)**: `#181a20` (Hover and active fills)
* **Border Default (`--border`)**: `#282b34` (1px clean separators)
* **Border Hover (`--border-hover`)**: `#3e4352`
* **Text Primary (`--text`)**: `#f4f4f6` (High contrast)
* **Text Muted (`--text-muted`)**: `#8e93a0` (Secondary labels & units)
* **Accent & Hardware**: Chrome, phosphor bronze bushings, carbon fiber dark frames, and cyan optical status glow (`#00e5ff`).

## 2. Layout & Viewport Hierarchy (Split Console)
* **Header (48px)**: Brand title, active DSL version, 18 GDL status badge, and global execution controls.
* **Main 3D Viewport**: Full-bleed WebGL canvas with orbit controls, floating camera reset/zoom shortcuts, and kinematic timeline overlay.
* **Collapsible Side Drawer (360px / 520px expanded)**:
  * **Tab 1 — Articulaciones (18 GDL)**: Individual range sliders for all 15 finger falanges (MCP, PIP, DIP) and 3 wrist axes (Pitch, Roll, Yaw), plus predefined biomechanical poses (Reposo, Puño, Pinza, Señalar).
  * **Tab 2 — Cuádruplas TAC**: Intermediate code representation table showing compiler operations in real time.
  * **Tab 3 — Sensor Óptico**: MediaPipe vision feed and landmark inference telemetry.
* **Activity Rail (48px)**: Minimal vertical icon strip on the far right for quick drawer toggling.

## 3. Typography & Components
* **UI Copy**: System sans-serif stack (`-apple-system`, `Segoe UI`, `Roboto`).
* **Telemetry & Code**: Monospaced font stack (`ui-monospace`, `SFMono-Regular`, `Consolas`).
* **Interactive Elements**: Micro-interactions with 120ms transitions, subtle scale transforms on slider thumbs, and accessible contrast ratios.
