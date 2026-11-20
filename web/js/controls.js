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
    const val = parseFloat(e.target.value);
    handState.wrist.target.pitch = THREE.MathUtils.degToRad(val);
    const lbl = document.getElementById('lbl-wrist-pitch');
    if (lbl) lbl.textContent = `${Math.round(val)}°`;
    markManualTuning();
  });
  if (srvRoll) srvRoll.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    handState.wrist.target.roll = THREE.MathUtils.degToRad(val);
    const lbl = document.getElementById('lbl-wrist-roll');
    if (lbl) lbl.textContent = `${Math.round(val)}°`;
    markManualTuning();
  });
  if (srvYaw) srvYaw.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    handState.wrist.target.yaw = THREE.MathUtils.degToRad(val);
    const lbl = document.getElementById('lbl-wrist-yaw');
    if (lbl) lbl.textContent = `${Math.round(val)}°`;
    markManualTuning();
  });
  if (srvArch) srvArch.addEventListener('input', (e) => {
    const val = parseFloat(e.target.value);
    handState.wrist.target.palmArch = THREE.MathUtils.degToRad(val);
    const lbl = document.getElementById('lbl-palm-arch');
    if (lbl) lbl.textContent = `${Math.round(val)}°`;
    markManualTuning();
  });
  if (srvSmooth) srvSmooth.addEventListener('input', (e) => {
    handState.smoothing = parseFloat(e.target.value);
    const valEl = document.getElementById('val-smoothing');
    if (valEl) valEl.textContent = handState.smoothing.toFixed(2);
  });
}

function markManualTuning() {
  document.querySelectorAll('.pose-chip').forEach(b => b.classList.remove('active'));
  const gBadge = document.getElementById('gesture-badge');
  if (gBadge && !handState.cameraActive && !trajectoryPlayback?.isPlaying) {
    gBadge.textContent = 'MANUAL / 18 GDL';
  }
}

function syncAllSlidersToTargets() {
  // 1. Sync Wrist (3 GDL + Arch)
  const wTar = handState.wrist.target;
  const pDeg = Math.round(THREE.MathUtils.radToDeg(wTar.pitch));
  const rDeg = Math.round(THREE.MathUtils.radToDeg(wTar.roll));
  const yDeg = Math.round(THREE.MathUtils.radToDeg(wTar.yaw));
  const aDeg = Math.round(THREE.MathUtils.radToDeg(wTar.palmArch));

  const srvP = document.getElementById('srv-wrist-pitch');
  const srvR = document.getElementById('srv-wrist-roll');
  const srvY = document.getElementById('srv-wrist-yaw');
  const srvA = document.getElementById('srv-palm-arch');
  if (srvP) srvP.value = pDeg;
  if (srvR) srvR.value = rDeg;
  if (srvY) srvY.value = yDeg;
  if (srvA) srvA.value = aDeg;

  const lblP = document.getElementById('lbl-wrist-pitch');
  const lblR = document.getElementById('lbl-wrist-roll');
  const lblY = document.getElementById('lbl-wrist-yaw');
  const lblA = document.getElementById('lbl-palm-arch');
  if (lblP) lblP.textContent = `${pDeg}°`;
  if (lblR) lblR.textContent = `${rDeg}°`;
  if (lblY) lblY.textContent = `${yDeg}°`;
  if (lblA) lblA.textContent = `${aDeg}°`;

  // 2. Sync Fingers (15 GDL + curl + splay)
  FINGER_CONFIGS.forEach(cfg => {
    const fTar = handState.fingers[cfg.id]?.target;
    if (!fTar) return;

    const mcpDeg = Math.round(THREE.MathUtils.radToDeg(fTar.mcp));
    const pipDeg = Math.round(THREE.MathUtils.radToDeg(fTar.pip));
    const dipDeg = Math.round(THREE.MathUtils.radToDeg(fTar.dip));
    let splayDeg = Math.round(THREE.MathUtils.radToDeg(fTar.splay));
    if (cfg.id === 'ring' || cfg.id === 'pinky') {
      splayDeg = -splayDeg;
    }

    const maxMcp = cfg.id === 'thumb' ? 1.05 : 1.52;
    const curlPct = Math.min(100, Math.max(0, Math.round((fTar.mcp / maxMcp) * 100)));

    const srvMcp   = document.getElementById(`srv-${cfg.id}-mcp`);
    const srvPip   = document.getElementById(`srv-${cfg.id}-pip`);
    const srvDip   = document.getElementById(`srv-${cfg.id}-dip`);
    const srvSplay = document.getElementById(`srv-${cfg.id}-splay`);
    const srvCurl  = document.getElementById(`srv-${cfg.id}-curl`);

    if (srvMcp)   srvMcp.value   = mcpDeg;
    if (srvPip)   srvPip.value   = pipDeg;
    if (srvDip)   srvDip.value   = dipDeg;
    if (srvSplay) srvSplay.value = splayDeg;
    if (srvCurl)  srvCurl.value  = curlPct;

    const lblMcp   = document.getElementById(`lbl-${cfg.id}-mcp`);
    const lblPip   = document.getElementById(`lbl-${cfg.id}-pip`);
    const lblDip   = document.getElementById(`lbl-${cfg.id}-dip`);
    const lblSplay = document.getElementById(`lbl-${cfg.id}-splay`);
    const lblCurl  = document.getElementById(`lbl-${cfg.id}-curl`);
    const lblTag   = document.getElementById(`lbl-${cfg.id}-tag`);

    if (lblMcp)   lblMcp.textContent   = `${mcpDeg}°`;
    if (lblPip)   lblPip.textContent   = `${pipDeg}°`;
    if (lblDip)   lblDip.textContent   = `${dipDeg}°`;
    if (lblSplay) lblSplay.textContent = `${splayDeg}°`;
    if (lblCurl)  lblCurl.textContent  = `${curlPct}%`;
    if (lblTag)   lblTag.textContent   = `MCP: ${mcpDeg}° • PIP: ${pipDeg}° • DIP: ${dipDeg}°`;
  });
}

function onFingerMasterCurl(fingerId, percentVal) {
  const norm = parseFloat(percentVal) / 100;
  const fTarget = handState.fingers[fingerId].target;
  const maxMcp = fingerId === 'thumb' ? 1.05 : 1.52;
  const maxPip = fingerId === 'thumb' ? 1.15 : 1.65;
  const maxDip = fingerId === 'thumb' ? 0.95 : 1.25;

  fTarget.mcp = norm * maxMcp;
  fTarget.pip = norm * maxPip;
  fTarget.dip = norm * maxDip;

  const lblCurl = document.getElementById(`lbl-${fingerId}-curl`);
  if (lblCurl) lblCurl.textContent = `${Math.round(percentVal)}%`;

  const mcpDeg = Math.round(THREE.MathUtils.radToDeg(fTarget.mcp));
  const pipDeg = Math.round(THREE.MathUtils.radToDeg(fTarget.pip));
  const dipDeg = Math.round(THREE.MathUtils.radToDeg(fTarget.dip));

  const srvMcp = document.getElementById(`srv-${fingerId}-mcp`);
  const srvPip = document.getElementById(`srv-${fingerId}-pip`);
  const srvDip = document.getElementById(`srv-${fingerId}-dip`);
  if (srvMcp) srvMcp.value = mcpDeg;
  if (srvPip) srvPip.value = pipDeg;
  if (srvDip) srvDip.value = dipDeg;

  const lblMcp = document.getElementById(`lbl-${fingerId}-mcp`);
  const lblPip = document.getElementById(`lbl-${fingerId}-pip`);
  const lblDip = document.getElementById(`lbl-${fingerId}-dip`);
  if (lblMcp) lblMcp.textContent = `${mcpDeg}°`;
  if (lblPip) lblPip.textContent = `${pipDeg}°`;
  if (lblDip) lblDip.textContent = `${dipDeg}°`;

  markManualTuning();
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
    markManualTuning();
    return;
  }

  handState.fingers[fingerId].target[jointKey] = rad;
  const lbl = document.getElementById(`lbl-${fingerId}-${jointKey}`);
  if (lbl) lbl.textContent = `${Math.round(val)}°`;

  // Update curl slider if MCP changed
  if (jointKey === 'mcp') {
    const maxMcp = fingerId === 'thumb' ? 1.05 : 1.52;
    const curlPct = Math.min(100, Math.max(0, Math.round((rad / maxMcp) * 100)));
    const srvCurl = document.getElementById(`srv-${fingerId}-curl`);
    const lblCurl = document.getElementById(`lbl-${fingerId}-curl`);
    if (srvCurl) srvCurl.value = curlPct;
    if (lblCurl) lblCurl.textContent = `${curlPct}%`;
  }

  markManualTuning();
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

  const lp = document.getElementById('lbl-wrist-pitch');
  const lr = document.getElementById('lbl-wrist-roll');
  const ly = document.getElementById('lbl-wrist-yaw');
  const la = document.getElementById('lbl-palm-arch');
  if (lp) lp.textContent = '0°';
  if (lr) lr.textContent = '0°';
  if (ly) ly.textContent = '0°';
  if (la) la.textContent = '5°';
}

function applyPreset(presetName, btn) {
  if (trajectoryPlayback?.isPlaying && typeof pauseTrajectory === 'function') {
    pauseTrajectory();
  }
  if (handState.cameraActive && typeof stopWebcamTracking === 'function') {
    stopWebcamTracking();
  }

  const p = (presetName || '').toLowerCase().trim();

  // Highlight matching chip
  document.querySelectorAll('.pose-chip').forEach(b => b.classList.remove('active'));
  const targetChip = btn || document.querySelector(`.pose-chip[data-preset="${p}"]`);
  if (targetChip) targetChip.classList.add('active');

  const setF = (id, mcpDeg, pipDeg, dipDeg, splay) => {
    handState.fingers[id].target.mcp = THREE.MathUtils.degToRad(mcpDeg);
    handState.fingers[id].target.pip = THREE.MathUtils.degToRad(pipDeg);
    handState.fingers[id].target.dip = THREE.MathUtils.degToRad(dipDeg);
    handState.fingers[id].target.splay = splay;
  };

  resetWrist();
  let badgeText = 'REPOSO';

  if (p === 'reposo' || p === 'rest') {
    setF('thumb', 6, 6, 4, 0.32);
    setF('index', 8, 6, 4, 0.18);
    setF('middle', 8, 6, 4, 0.0);
    setF('ring', 8, 6, 4, -0.14);
    setF('pinky', 10, 8, 6, -0.28);
    handState.wrist.target.palmArch = 0.08;
    badgeText = 'REPOSO';
  } else if (p === 'abierta' || p === 'open' || p === 'neutral') {
    setF('thumb', 0, 0, 0, 0.38);
    setF('index', 0, 0, 0, 0.22);
    setF('middle', 0, 0, 0, 0.0);
    setF('ring', 0, 0, 0, -0.18);
    setF('pinky', 0, 0, 0, -0.32);
    handState.wrist.target.palmArch = 0.02;
    badgeText = 'MANO ABIERTA';
  } else if (p === 'fist' || p === 'puño' || p === 'puno') {
    setF('thumb', 55, 70, 50, -0.25);
    setF('index', 85, 88, 75, 0.0);
    setF('middle', 88, 90, 75, 0.0);
    setF('ring', 88, 90, 75, 0.0);
    setF('pinky', 88, 90, 75, 0.0);
    handState.wrist.target.pitch = THREE.MathUtils.degToRad(10);
    handState.wrist.target.palmArch = 0.25;
    badgeText = 'PUÑO CERRADO';
  } else if (p === 'pinza' || p === 'pinch') {
    setF('thumb', 45, 55, 40, -0.20);
    setF('index', 50, 65, 45, -0.05);
    setF('middle', 75, 80, 60, 0.0);
    setF('ring', 80, 85, 65, -0.04);
    setF('pinky', 80, 85, 65, -0.08);
    handState.wrist.target.palmArch = 0.16;
    badgeText = 'PINZA DE PRECISIÓN';
  } else if (p === 'paz' || p === 'peace' || p === 'victoria') {
    setF('thumb', 55, 65, 45, -0.22);
    setF('index', 0, 0, 0, 0.25);
    setF('middle', 0, 0, 0, -0.10);
    setF('ring', 85, 90, 75, 0.0);
    setF('pinky', 85, 90, 75, 0.0);
    handState.wrist.target.palmArch = 0.08;
    badgeText = 'PAZ / VICTORIA';
  } else if (p === 'ok') {
    setF('thumb', 48, 58, 42, -0.22);
    setF('index', 52, 68, 46, -0.06);
    setF('middle', 4, 4, 4, 0.08);
    setF('ring', 6, 6, 4, -0.06);
    setF('pinky', 8, 8, 6, -0.16);
    handState.wrist.target.pitch = THREE.MathUtils.degToRad(-5);
    handState.wrist.target.palmArch = 0.14;
    badgeText = 'SIGNO OK';
  } else if (p === 'point' || p === 'señalar' || p === 'senalar' || p === 'apuntar') {
    setF('thumb', 50, 60, 40, -0.20);
    setF('index', 0, 0, 0, 0.05);
    setF('middle', 85, 90, 75, 0.0);
    setF('ring', 85, 90, 75, 0.0);
    setF('pinky', 85, 90, 75, 0.0);
    handState.wrist.target.pitch = THREE.MathUtils.degToRad(5);
    handState.wrist.target.palmArch = 0.10;
    badgeText = 'SEÑALAR';
  }

  // Synchronize all 18 GDL sliders + labels to the target pose
  syncAllSlidersToTargets();

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
