/**
 * materials.js — Three.js Robot Materials & Shading Management
 */

const materials = {};

function createRobotMaterials() {
  // 1. Primary Structural Shell: Matte Sandblasted Dark Titanium
  materials.armor = new THREE.MeshStandardMaterial({
    color: 0x1f232b,
    metalness: 0.82,
    roughness: 0.32
  });

  // 2. Skeletal Frame / Internal Core: Matte Carbon-Fiber Polymer
  materials.darkFrame = new THREE.MeshStandardMaterial({
    color: 0x111317,
    metalness: 0.40,
    roughness: 0.58
  });

  // 3. Precision Pivots, Axles & Hydraulic Rams: Mirror Hard Chrome Steel
  materials.jointChrome = new THREE.MeshStandardMaterial({
    color: 0xdde3ec,
    metalness: 0.96,
    roughness: 0.14
  });

  // 4. Low-Friction Bushings & Bearing Collars: Sintered Phosphor Bronze
  materials.bronzeBushing = new THREE.MeshStandardMaterial({
    color: 0xd4a359,
    metalness: 0.88,
    roughness: 0.28
  });

  // 5. Tactile Compliance Pads & Friction Tread: Vulcanized Nitrile Elastomer
  materials.sensorPad = new THREE.MeshStandardMaterial({
    color: 0x0c0e12,
    metalness: 0.08,
    roughness: 0.92
  });

  // 6. Subdued Telemetry Optics & Optical Encoders: Ice Cyan Luminescent
  materials.ledGlow = new THREE.MeshStandardMaterial({
    color: 0x38bdf8,
    emissive: 0x0284c7,
    emissiveIntensity: 0.85,
    roughness: 0.2
  });

  // 7. Synthetic Tendons / Micro-Cables: Braided Tungsten-Dyneema
  materials.tendonWire = new THREE.MeshStandardMaterial({
    color: 0x71717a,
    metalness: 0.85,
    roughness: 0.35
  });

  // 8. Hardware Fasteners & Retaining Caps: Blued Alloy Steel
  materials.fastenerAlloy = new THREE.MeshStandardMaterial({
    color: 0x27272a,
    metalness: 0.92,
    roughness: 0.3
  });
}

function setShadingMode(mode, btn) {
  document.querySelectorAll('.viewport-top-right .view-control-btn').forEach(b => b.classList.remove('active'));
  const activeBtn = btn || (mode === 'wire' ? document.getElementById('btn-wire') : document.getElementById('btn-solid'));
  if (activeBtn) activeBtn.classList.add('active');
  handState.wireframe = (mode === 'wire');
  Object.values(materials).forEach(mat => {
    if (mat && 'wireframe' in mat) mat.wireframe = handState.wireframe;
  });
}

function toggleWireframe() {
  const isWire = !handState.wireframe;
  const btn = isWire ? document.getElementById('btn-wire') : document.getElementById('btn-solid');
  setShadingMode(isWire ? 'wire' : 'solid', btn);
}
