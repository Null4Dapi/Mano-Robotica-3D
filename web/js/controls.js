/**
 * controls.js — UI Drawer Management, 18 GDL Manual Sliders & Telemetry
 */

let currentActiveRail = null;

function toggleRail(paneId) {
  const drawer = document.getElementById('side-drawer');
  const titleEl = document.getElementById('drawer-title');

  if (currentActiveRail === paneId && !drawer.classList.contains('collapsed')) {
    closeDrawer();
    return;
  }

  currentActiveRail = paneId;
  drawer.classList.remove('collapsed');

  document.querySelectorAll('.tab-pane').forEach(p => p.classList.remove('active'));
  document.querySelectorAll('.rail-btn').forEach(b => b.classList.remove('active'));

  const activePane = document.getElementById(`pane-${paneId}`);
  if (activePane) activePane.classList.add('active');

  const activeBtn = document.getElementById(`btn-rail-${paneId}`);
  if (activeBtn) activeBtn.classList.add('active');

  if (paneId === 'servos') {
    titleEl.textContent = 'Articulaciones (18 GDL)';
  } else if (paneId === 'trayectoria') {
    titleEl.textContent = 'Cuádruplas TAC / Trayectoria';
  } else if (paneId === 'sensor') {
    titleEl.textContent = 'Sensor Óptico MediaPipe';
  }

  setTimeout(onWindowResize, 240);
}

function toggleDrawerWidth() {
  const drawer = document.getElementById('side-drawer');
  drawer.classList.toggle('expanded');
  setTimeout(onWindowResize, 240);
}

function closeDrawer() {
  const drawer = document.getElementById('side-drawer');
  drawer.classList.add('collapsed');
  drawer.classList.remove('expanded');
  currentActiveRail = null;
  document.querySelectorAll('.rail-btn').forEach(b => b.classList.remove('active'));
  setTimeout(onWindowResize, 240);
}

function buildServoSlidersUI() {
  const container = document.getElementById('fingers-controls-container');
  if (!container) return;
  container.innerHTML = '';

  const fingerLabels = {
    thumb: 'Dedo Pulgar',
    index: 'Dedo Índice',
    middle: 'Dedo Medio',
    ring: 'Dedo Anular',
    pinky: 'Dedo Meñique'
  };

  FINGER_CONFIGS.forEach(cfg => {
    const card = document.createElement('div');
    card.className = 'finger-card';
    card.innerHTML = `
      <div class="finger-card-header">
        <span class="finger-title">${fingerLabels[cfg.id] || cfg.name}</span>
        <span id="lbl-${cfg.id}-tag" class="finger-angle-tag">MCP: 0° • PIP: 0° • DIP: 0°</span>
      </div>
      <div class="joint-grid">
        <span title="Flexión conjunta proporcional">Curl</span>
        <input id="srv-${cfg.id}-curl" type="range" min="0" max="100" value="6"
          oninput="onFingerMasterCurl('${cfg.id}', this.value)">
        <span id="lbl-${cfg.id}-curl">6%</span>
      </div>
      <div class="joint-grid">
        <span title="Falange Proximal (Nudillo MCP) [0° a 90°]">MCP</span>
        <input id="srv-${cfg.id}-mcp" type="range" min="0" max="90" value="6"
          oninput="onJointManualChange('${cfg.id}', 'mcp', this.value)">
        <span id="lbl-${cfg.id}-mcp">6°</span>
      </div>
      <div class="joint-grid">
        <span title="Falange Medial (Articulación PIP) [0° a 90°]">PIP</span>
        <input id="srv-${cfg.id}-pip" type="range" min="0" max="90" value="4"
          oninput="onJointManualChange('${cfg.id}', 'pip', this.value)">
        <span id="lbl-${cfg.id}-pip">4°</span>
      </div>
      <div class="joint-grid">
        <span title="Falange Distal (Punta DIP) [0° a 90°]">DIP</span>
        <input id="srv-${cfg.id}-dip" type="range" min="0" max="90" value="3"
          oninput="onJointManualChange('${cfg.id}', 'dip', this.value)">
        <span id="lbl-${cfg.id}-dip">3°</span>
      </div>
      <div class="joint-grid">
        <span title="Apertura / Abducción lateral">Apertura</span>
        <input id="srv-${cfg.id}-splay" type="range" min="-25" max="25" value="0"
          oninput="onJointManualChange('${cfg.id}', 'splay', this.value)">
        <span id="lbl-${cfg.id}-splay">0°</span>
      </div>
    `;
    container.appendChild(card);
  });

  // Wrist listeners
  const srvPitch = document.getElementById('srv-wrist-pitch');
  const srvRoll  = document.getElementById('srv-wrist-roll');
  const srvYaw   = document.getElementById('srv-wrist-yaw');
  const srvArch  = document.getElementById('srv-palm-arch');
  const srvSmooth = document.getElementById('slider-smoothing');

  if (srvPitch) srvPitch.addEventListener('input', (e) => {
    handState.wrist.target.pitch = THREE.MathUtils.degToRad(parseFloat(e.target.value));
  });
  if (srvRoll) srvRoll.addEventListener('input', (e) => {
    handState.wrist.target.roll = THREE.MathUtils.degToRad(parseFloat(e.target.value));
  });
  if (srvYaw) srvYaw.addEventListener('input', (e) => {
    handState.wrist.target.yaw = THREE.MathUtils.degToRad(parseFloat(e.target.value));
  });
  if (srvArch) srvArch.addEventListener('input', (e) => {
    handState.wrist.target.palmArch = THREE.MathUtils.degToRad(parseFloat(e.target.value));
  });
  if (srvSmooth) srvSmooth.addEventListener('input', (e) => {
    handState.smoothing = parseFloat(e.target.value);
    const valEl = document.getElementById('val-smoothing');
    if (valEl) valEl.textContent = handState.smoothing.toFixed(2);
  });
}

function onFingerMasterCurl(fingerId, percentVal) {
  const norm = parseFloat(percentVal) / 100;
  const fTarget = handState.fingers[fingerId].target;
  if (fingerId === 'thumb') {
    fTarget.mcp = norm * 1.05;
    fTarget.pip = norm * 1.15;
    fTarget.dip = norm * 0.95;
  } else {
    fTarget.mcp = norm * 1.52;
    fTarget.pip = norm * 1.65;
    fTarget.dip = norm * 1.25;
  }
}

function onJointManualChange(fingerId, jointKey, degVal) {
  const val = parseFloat(degVal);
  const rad = THREE.MathUtils.degToRad(val);

  if (jointKey === 'splay') {
    let splayAngle = rad;
    if (fingerId === 'ring' || fingerId === 'pinky') {
      splayAngle = -rad;
    }
    handState.fingers[fingerId].target.splay = splayAngle;
    const lbl = document.getElementById(`lbl-${fingerId}-splay`);
    if (lbl) lbl.textContent = `${Math.round(val)}°`;
    return;
  }

  handState.fingers[fingerId].target[jointKey] = rad;
  const lbl = document.getElementById(`lbl-${fingerId}-${jointKey}`);
  if (lbl) lbl.textContent = `${Math.round(val)}°`;
}

function resetWrist() {
  handState.wrist.target = { pitch: 0, roll: 0, yaw: 0, palmArch: 0.08 };
  const p = document.getElementById('srv-wrist-pitch');
  const r = document.getElementById('srv-wrist-roll');
  const y = document.getElementById('srv-wrist-yaw');
  const a = document.getElementById('srv-palm-arch');
  if (p) p.value = 0;
  if (r) r.value = 0;
  if (y) y.value = 0;
  if (a) a.value = 5;
}

function applyPreset(presetName, btn) {
  if (btn) {
    document.querySelectorAll('.pose-chip').forEach(b => b.classList.remove('active'));
    btn.classList.add('active');
  }

  const setF = (id, curl, splay = 0) => {
    const isT = id === 'thumb';
    handState.fingers[id].target.mcp = curl * (isT ? 1.05 : 1.52);
    handState.fingers[id].target.pip = curl * (isT ? 1.15 : 1.65);
    handState.fingers[id].target.dip = curl * (isT ? 0.95 : 1.25);
    handState.fingers[id].target.splay = splay;
  };

  resetWrist();
  let badgeText = 'REPOSO / MANUAL';
  const p = (presetName || '').toLowerCase();

  if (p === 'open' || p === 'neutral' || p === 'reposo') {
    setF('thumb', 0.04, 0.32);
    setF('index', 0.04, 0.18);
    setF('middle', 0.04, 0.0);
    setF('ring', 0.04, -0.14);
    setF('pinky', 0.04, -0.28);
    handState.wrist.target.palmArch = 0.04;
    badgeText = 'REPOSO / ABIERTA';
  } else if (p === 'fist' || p === 'puño' || p === 'puno') {
    setF('thumb', 0.92, -0.25);
    setF('index', 0.98, 0);
    setF('middle', 0.98, 0);
    setF('ring', 0.98, 0);
    setF('pinky', 0.98, 0);
    handState.wrist.target.palmArch = 0.32;
    badgeText = 'PUÑO CERRADO';
  } else if (p === 'ok' || p === 'pinch' || p === 'pinza') {
    setF('thumb', 0.72, -0.22);
    setF('index', 0.74, -0.06);
    setF('middle', 0.08, 0.05);
    setF('ring', 0.06, -0.05);
    setF('pinky', 0.05, -0.12);
    handState.wrist.target.palmArch = 0.20;
    badgeText = 'PINZA OK';
  } else if (p === 'point' || p === 'señalar' || p === 'senalar' || p === 'apuntar') {
    setF('thumb', 0.65, -0.2);
    setF('index', 0.03, 0);
    setF('middle', 0.94, 0);
    setF('ring', 0.95, 0);
    setF('pinky', 0.95, 0);
    badgeText = 'SEÑALAR';
  }

  const gBadge = document.getElementById('gesture-badge');
  if (gBadge) gBadge.textContent = badgeText;
}

function updateWristTelemetryUI(wCur) {
  const pitchDeg = Math.round(THREE.MathUtils.radToDeg(wCur.pitch));
  const rollDeg  = Math.round(THREE.MathUtils.radToDeg(wCur.roll));
  const yawDeg   = Math.round(THREE.MathUtils.radToDeg(wCur.yaw));
  const archDeg  = Math.round(THREE.MathUtils.radToDeg(wCur.palmArch));

  const lblPitch = document.getElementById('lbl-wrist-pitch');
  const lblRoll  = document.getElementById('lbl-wrist-roll');
  const lblYaw   = document.getElementById('lbl-wrist-yaw');
  const lblArch  = document.getElementById('lbl-palm-arch');

  if (lblPitch) lblPitch.textContent = `${pitchDeg}°`;
  if (lblRoll)  lblRoll.textContent  = `${rollDeg}°`;
  if (lblYaw)   lblYaw.textContent   = `${yawDeg}°`;
  if (lblArch)  lblArch.textContent  = `${archDeg}°`;

  if (handState.cameraActive || trajectoryPlayback.isPlaying) {
    const srvPitch = document.getElementById('srv-wrist-pitch');
    const srvRoll  = document.getElementById('srv-wrist-roll');
    const srvYaw   = document.getElementById('srv-wrist-yaw');
    const srvArch  = document.getElementById('srv-palm-arch');
    if (srvPitch) srvPitch.value = pitchDeg;
    if (srvRoll)  srvRoll.value  = rollDeg;
    if (srvYaw)   srvYaw.value   = yawDeg;
    if (srvArch)  srvArch.value  = archDeg;
  }
}

function updateFingerTelemetryUI(id, cur) {
  const maxMcp = id === 'thumb' ? 1.05 : 1.52;
  const curlPct = Math.min(100, Math.max(0, Math.round((cur.mcp / maxMcp) * 100)));
  const mcpDeg  = Math.round(THREE.MathUtils.radToDeg(cur.mcp));
  const pipDeg  = Math.round(THREE.MathUtils.radToDeg(cur.pip));
  const dipDeg  = Math.round(THREE.MathUtils.radToDeg(cur.dip));
  const splayDeg = Math.round(THREE.MathUtils.radToDeg(cur.splay));

  const lblCurl  = document.getElementById(`lbl-${id}-curl`);
  const lblMcp   = document.getElementById(`lbl-${id}-mcp`);
  const lblPip   = document.getElementById(`lbl-${id}-pip`);
  const lblDip   = document.getElementById(`lbl-${id}-dip`);
  const lblSplay = document.getElementById(`lbl-${id}-splay`);
  const lblTag   = document.getElementById(`lbl-${id}-tag`);

  if (lblCurl)  lblCurl.textContent  = `${curlPct}%`;
  if (lblMcp)   lblMcp.textContent   = `${mcpDeg}°`;
  if (lblPip)   lblPip.textContent   = `${pipDeg}°`;
  if (lblDip)   lblDip.textContent   = `${dipDeg}°`;
  if (lblSplay) lblSplay.textContent = `${splayDeg}°`;
  if (lblTag)   lblTag.textContent   = `MCP: ${mcpDeg}° • PIP: ${pipDeg}° • DIP: ${dipDeg}°`;

  if (handState.cameraActive || trajectoryPlayback.isPlaying) {
    const srvCurl  = document.getElementById(`srv-${id}-curl`);
    const srvMcp   = document.getElementById(`srv-${id}-mcp`);
    const srvPip   = document.getElementById(`srv-${id}-pip`);
    const srvDip   = document.getElementById(`srv-${id}-dip`);
    const srvSplay = document.getElementById(`srv-${id}-splay`);
    if (srvCurl)  srvCurl.value  = curlPct;
    if (srvMcp)   srvMcp.value   = mcpDeg;
    if (srvPip)   srvPip.value   = pipDeg;
    if (srvDip)   srvDip.value   = dipDeg;
    if (srvSplay) srvSplay.value = splayDeg;
  }
}

function updateStatusMessage(msg) {
  const el = document.getElementById('status-message');
  if (el) el.textContent = msg;
}
