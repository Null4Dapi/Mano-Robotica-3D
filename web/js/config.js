/**
 * config.js — Biomechanical Configuration & Kinematic State (18 GDL)
 */

const FINGER_CONFIGS = [
  {
    id: 'thumb',
    name: 'Pulgar',
    isThumb: true,
    basePos: { x: -0.92, y: 0.60, z: 0.20 },
    baseRot: { x: 0.24, y: 0.48, z: 0.36 },
    lengths: [0.72, 0.60, 0.48],
    radii: [0.29, 0.25, 0.21]
  },
  {
    id: 'index',
    name: 'Índice',
    isThumb: false,
    basePos: { x: -0.95, y: 2.15, z: 0.08 },
    baseRot: { x: 0, y: 0, z: 0.06 },
    lengths: [1.18, 0.84, 0.62],
    radii: [0.27, 0.24, 0.20]
  },
  {
    id: 'middle',
    name: 'Medio',
    isThumb: false,
    basePos: { x: -0.28, y: 2.28, z: 0.0 },
    baseRot: { x: 0, y: 0, z: 0.0 },
    lengths: [1.28, 0.92, 0.66],
    radii: [0.28, 0.25, 0.21]
  },
  {
    id: 'ring',
    name: 'Anular',
    isThumb: false,
    basePos: { x: 0.38, y: 2.16, z: 0.06 },
    baseRot: { x: 0, y: 0, z: -0.05 },
    lengths: [1.16, 0.85, 0.62],
    radii: [0.26, 0.23, 0.19]
  },
  {
    id: 'pinky',
    name: 'Meñique',
    isThumb: false,
    basePos: { x: 1.02, y: 1.92, z: 0.14 },
    baseRot: { x: 0, y: 0, z: -0.12 },
    lengths: [0.92, 0.66, 0.52],
    radii: [0.23, 0.20, 0.17]
  }
];

const handState = {
  smoothing: 0.22,
  cameraActive: false,
  mirrorWebcam: true,
  wireframe: false,
  wrist: {
    current: { pitch: 0, roll: 0, yaw: 0, palmArch: 0.08 },
    target:  { pitch: 0, roll: 0, yaw: 0, palmArch: 0.08 }
  },
  fingers: {}
};

// Initialize finger states
FINGER_CONFIGS.forEach(cfg => {
  let initSplay = 0;
  if (cfg.id === 'thumb')  initSplay =  0.35;
  if (cfg.id === 'index')  initSplay =  0.22;
  if (cfg.id === 'middle') initSplay =  0.0;
  if (cfg.id === 'ring')   initSplay = -0.18;
  if (cfg.id === 'pinky')  initSplay = -0.32;

  handState.fingers[cfg.id] = {
    current: { mcp: 0.08, pip: 0.06, dip: 0.04, splay: initSplay },
    target:  { mcp: 0.08, pip: 0.06, dip: 0.04, splay: initSplay }
  };
});

const PRESET_POSES = {
  reposo: {
    label: 'REPOSO',
    wrist: { pitch: 0, roll: 0, yaw: 0, palmArch: 0.08 },
    fingers: {
      thumb:  { mcp: 0.10, pip: 0.10, dip: 0.07, splay:  0.32 },
      index:  { mcp: 0.14, pip: 0.10, dip: 0.07, splay:  0.18 },
      middle: { mcp: 0.14, pip: 0.10, dip: 0.07, splay:  0.0  },
      ring:   { mcp: 0.14, pip: 0.10, dip: 0.07, splay: -0.14 },
      pinky:  { mcp: 0.17, pip: 0.14, dip: 0.10, splay: -0.28 }
    }
  },
  abierta: {
    label: 'MANO ABIERTA',
    wrist: { pitch: 0, roll: 0, yaw: 0, palmArch: 0.02 },
    fingers: {
      thumb:  { mcp: 0.0, pip: 0.0, dip: 0.0, splay:  0.38 },
      index:  { mcp: 0.0, pip: 0.0, dip: 0.0, splay:  0.22 },
      middle: { mcp: 0.0, pip: 0.0, dip: 0.0, splay:  0.0  },
      ring:   { mcp: 0.0, pip: 0.0, dip: 0.0, splay: -0.18 },
      pinky:  { mcp: 0.0, pip: 0.0, dip: 0.0, splay: -0.32 }
    }
  },
  fist: {
    label: 'PUÑO CERRADO',
    wrist: { pitch: 0.17, roll: 0, yaw: 0, palmArch: 0.25 },
    fingers: {
      thumb:  { mcp: 0.96, pip: 1.22, dip: 0.87, splay: -0.25 },
      index:  { mcp: 1.48, pip: 1.54, dip: 1.31, splay:  0.0  },
      middle: { mcp: 1.54, pip: 1.57, dip: 1.31, splay:  0.0  },
      ring:   { mcp: 1.54, pip: 1.57, dip: 1.31, splay:  0.0  },
      pinky:  { mcp: 1.54, pip: 1.57, dip: 1.31, splay:  0.0  }
    }
  },
  pinza: {
    label: 'PINZA DE PRECISIÓN',
    wrist: { pitch: 0, roll: 0, yaw: 0, palmArch: 0.16 },
    fingers: {
      thumb:  { mcp: 0.78, pip: 0.96, dip: 0.70, splay: -0.20 },
      index:  { mcp: 0.87, pip: 1.13, dip: 0.79, splay: -0.05 },
      middle: { mcp: 1.31, pip: 1.40, dip: 1.05, splay:  0.0  },
      ring:   { mcp: 1.40, pip: 1.48, dip: 1.13, splay: -0.04 },
      pinky:  { mcp: 1.40, pip: 1.48, dip: 1.13, splay: -0.08 }
    }
  },
  paz: {
    label: 'PAZ / VICTORIA',
    wrist: { pitch: 0, roll: 0, yaw: 0, palmArch: 0.08 },
    fingers: {
      thumb:  { mcp: 0.96, pip: 1.13, dip: 0.79, splay: -0.22 },
      index:  { mcp: 0.0,  pip: 0.0,  dip: 0.0,  splay:  0.25 },
      middle: { mcp: 0.0,  pip: 0.0,  dip: 0.0,  splay: -0.10 },
      ring:   { mcp: 1.48, pip: 1.57, dip: 1.31, splay:  0.0  },
      pinky:  { mcp: 1.48, pip: 1.57, dip: 1.31, splay:  0.0  }
    }
  },
  ok: {
    label: 'SIGNO OK',
    wrist: { pitch: -0.09, roll: 0, yaw: 0, palmArch: 0.14 },
    fingers: {
      thumb:  { mcp: 0.84, pip: 1.01, dip: 0.73, splay: -0.22 },
      index:  { mcp: 0.91, pip: 1.19, dip: 0.80, splay: -0.06 },
      middle: { mcp: 0.07, pip: 0.07, dip: 0.07, splay:  0.08 },
      ring:   { mcp: 0.10, pip: 0.10, dip: 0.07, splay: -0.06 },
      pinky:  { mcp: 0.14, pip: 0.14, dip: 0.10, splay: -0.16 }
    }
  },
  point: {
    label: 'SEÑALAMIENTO / GESTO ÍNDICE',
    wrist: { pitch: 0.09, roll: 0, yaw: 0, palmArch: 0.10 },
    fingers: {
      thumb:  { mcp: 0.87, pip: 1.05, dip: 0.70, splay: -0.20 },
      index:  { mcp: 0.0,  pip: 0.0,  dip: 0.0,  splay:  0.05 },
      middle: { mcp: 1.48, pip: 1.57, dip: 1.31, splay:  0.0  },
      ring:   { mcp: 1.48, pip: 1.57, dip: 1.31, splay:  0.0  },
      pinky:  { mcp: 1.48, pip: 1.57, dip: 1.31, splay:  0.0  }
    }
  }
};

// Backward compatibility aliases
PRESET_POSES.open = PRESET_POSES.abierta;
PRESET_POSES.puño = PRESET_POSES.fist;
