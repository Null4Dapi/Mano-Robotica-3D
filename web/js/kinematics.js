/**
 * kinematics.js — Kinematic Calculations, Joint Angle Interpolation & Trajectory Playback (18 GDL)
 */

let loadedTrajectory = null;
let trajectoryPlayback = {
  isPlaying: false,
  frameIdx: 0,
  timer: null,
  speed: 1.0
};

function applyTrajectoryJoints(joints) {
  if (!joints) return;

  // 1. Wrist Orientation (3 GDL)
  if (joints.MUNICA_PITCH !== undefined) handState.wrist.target.pitch = THREE.MathUtils.degToRad(joints.MUNICA_PITCH);
  if (joints.MUNICA_ROLL !== undefined)  handState.wrist.target.roll  = THREE.MathUtils.degToRad(joints.MUNICA_ROLL);
  if (joints.MUNICA_YAW !== undefined)   handState.wrist.target.yaw   = THREE.MathUtils.degToRad(joints.MUNICA_YAW);

  // 2. Finger Phalanxes (15 GDL: 5 fingers × 3 joints)
  const fMap = {
    'PULGAR': 'thumb',
    'INDICE': 'index',
    'MEDIO': 'middle',
    'ANULAR': 'ring',
    'MENIQUE': 'pinky'
  };

  for (const [prefix, id] of Object.entries(fMap)) {
    const target = handState.fingers[id]?.target;
    if (!target) continue;
    if (joints[`${prefix}_PROXIMAL`] !== undefined) {
      target.mcp = THREE.MathUtils.degToRad(joints[`${prefix}_PROXIMAL`]);
    }
    if (joints[`${prefix}_MEDIAL`] !== undefined) {
      target.pip = THREE.MathUtils.degToRad(joints[`${prefix}_MEDIAL`]);
    }
    if (joints[`${prefix}_DISTAL`] !== undefined) {
      target.dip = THREE.MathUtils.degToRad(joints[`${prefix}_DISTAL`]);
    }
  }

  // Dynamic Abduction/Splay Coupling during Trajectory Playback
  const idxProx = joints.INDICE_PROXIMAL || 0;
  const pinkProx = joints.MENIQUE_PROXIMAL || 0;
  const isFist = (idxProx > 30 && pinkProx > 30);
  const isPinch = (idxProx > 30 && pinkProx < 15);

  if (isFist) {
    handState.fingers.thumb.target.splay = -0.22;
    handState.fingers.index.target.splay = 0.02;
    handState.fingers.middle.target.splay = 0.0;
    handState.fingers.ring.target.splay = -0.02;
    handState.fingers.pinky.target.splay = -0.04;
  } else if (isPinch) {
    handState.fingers.thumb.target.splay = -0.18;
    handState.fingers.index.target.splay = -0.06;
    handState.fingers.middle.target.splay = 0.06;
    handState.fingers.ring.target.splay = -0.08;
    handState.fingers.pinky.target.splay = -0.16;
  } else {
    // Natural resting abanico
    handState.fingers.thumb.target.splay = 0.35;
    handState.fingers.index.target.splay = 0.22;
    handState.fingers.middle.target.splay = 0.0;
    handState.fingers.ring.target.splay = -0.18;
    handState.fingers.pinky.target.splay = -0.32;
  }
}

function setupTrajectoryData(data, autoPlay = false) {
  if (!data || !data.frames || data.frames.length === 0) {
    updateStatusMessage('Error: La trayectoria no contiene frames cinemáticos.');
    return;
  }
  loadedTrajectory = data;
  const totalFrames = data.frames.length;

  const range = document.getElementById('timeline-range');
  if (range) {
    range.min = 0;
    range.max = totalFrames - 1;
    range.value = 0;
  }

  applyTrajectoryFrame(0);
  updateStatusMessage(`Trayectoria cargada: ${data.programa || 'ManoBionica'} (${totalFrames} frames, ${data.duracion_total_ms || 0}ms)`);

  if (autoPlay) {
    resumeTrajectory();
  }
}

function applyTrajectoryFrame(idx) {
  if (!loadedTrajectory || !loadedTrajectory.frames || !loadedTrajectory.frames[idx]) return;
  const f = loadedTrajectory.frames[idx];
  applyTrajectoryJoints(f.articulaciones);
  trajectoryPlayback.frameIdx = idx;

  const range = document.getElementById('timeline-range');
  if (range) range.value = idx;

  const counter = document.getElementById('frame-counter');
  if (counter) {
    const secs = (f.tiempo_ms / 1000).toFixed(1);
    counter.textContent = `Frame ${idx + 1} / ${loadedTrajectory.frames.length} (${secs}s)`;
  }

  const gBadge = document.getElementById('gesture-badge');
  if (gBadge) {
    gBadge.textContent = `TRAYECTORIA [${idx + 1}/${loadedTrajectory.frames.length}]`;
  }
}

function stepTrajectory() {
  if (!trajectoryPlayback.isPlaying || !loadedTrajectory) return;
  const frames = loadedTrajectory.frames;
  if (!frames || frames.length === 0) return;

  const idx = trajectoryPlayback.frameIdx;
  applyTrajectoryFrame(idx);

  if (idx + 1 < frames.length) {
    const currentMs = frames[idx].tiempo_ms;
    const nextMs = frames[idx + 1].tiempo_ms;
    const diff = Math.max(20, nextMs - currentMs);
    const delay = Math.max(15, diff / trajectoryPlayback.speed);
    trajectoryPlayback.frameIdx++;
    trajectoryPlayback.timer = setTimeout(stepTrajectory, delay);
  } else {
    updateStatusMessage(`Trayectoria completada (${frames.length} frames).`);
    pauseTrajectory();
  }
}

function toggleTrajectoryPlayback() {
  if (!loadedTrajectory) {
    fetchTrajectoryJson(true);
    return;
  }
  if (trajectoryPlayback.isPlaying) {
    pauseTrajectory();
  } else {
    resumeTrajectory();
  }
}

function pauseTrajectory() {
  trajectoryPlayback.isPlaying = false;
  if (trajectoryPlayback.timer) clearTimeout(trajectoryPlayback.timer);
  updatePlayIcon(false);
  updateStatusMessage('Reproducción de trayectoria pausada.');
}

function resumeTrajectory() {
  if (handState.cameraActive && typeof stopWebcamTracking === 'function') {
    stopWebcamTracking();
  }
  trajectoryPlayback.isPlaying = true;
  updatePlayIcon(true);
  if (loadedTrajectory && trajectoryPlayback.frameIdx >= loadedTrajectory.frames.length - 1) {
    trajectoryPlayback.frameIdx = 0;
  }
  stepTrajectory();
}

function stopTrajectory() {
  pauseTrajectory();
  trajectoryPlayback.frameIdx = 0;
  applyTrajectoryFrame(0);
}

function updatePlayIcon(isPlaying) {
  const icon = document.getElementById('play-icon');
  if (!icon) return;
  if (isPlaying) {
    icon.innerHTML = '<rect x="5" y="4" width="4" height="16"/><rect x="15" y="4" width="4" height="16"/>';
  } else {
    icon.innerHTML = '<polygon points="5 3 19 12 5 21 5 3"/>';
  }
}

function onTimelineScrub(val) {
  const idx = parseInt(val, 10);
  applyTrajectoryFrame(idx);
}

function stepTimelineFrame(delta) {
  if (!loadedTrajectory || !loadedTrajectory.frames) return;
  const newIdx = THREE.MathUtils.clamp(trajectoryPlayback.frameIdx + delta, 0, loadedTrajectory.frames.length - 1);
  applyTrajectoryFrame(newIdx);
}

function cycleTrajectorySpeed(btn) {
  const speeds = [0.5, 1.0, 2.0];
  let curIdx = speeds.indexOf(trajectoryPlayback.speed);
  if (curIdx === -1) curIdx = 1;
  const nextIdx = (curIdx + 1) % speeds.length;
  trajectoryPlayback.speed = speeds[nextIdx];
  if (btn) btn.textContent = `${trajectoryPlayback.speed.toFixed(1)}x`;
}

async function fetchTrajectoryJson(autoPlay = false) {
  updateStatusMessage('Obteniendo trayectoria.json...');
  try {
    const resp = await fetch('trayectoria.json?nocache=' + Date.now(), { cache: 'no-store' });
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    const data = await resp.json();
    setupTrajectoryData(data, autoPlay);
  } catch (err) {
    console.warn('Fetch fallback:', err);
    updateStatusMessage('No se pudo descargar trayectoria.json vía HTTP. Use "Subir Archivo".');
  }
}

function onTrajectoryFileSelected(event) {
  const file = event.target.files?.[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = (e) => {
    try {
      const data = JSON.parse(e.target.result);
      setupTrajectoryData(data, true);
    } catch (err) {
      console.error('Error parseando JSON de trayectoria:', err);
      updateStatusMessage('Error al interpretar el archivo JSON de trayectoria.');
    }
  };
  reader.readAsText(file);
}

function reloadRobFile() {
  updateStatusMessage('Recargando archivo .rob y actualizando cinemática...');
  fetchTrajectoryJson(false);
}

function applyHandKinematics() {
  const alpha = handState.smoothing;

  // 1. Interpolate Wrist
  const wCur = handState.wrist.current;
  const wTar = handState.wrist.target;
  wCur.pitch    += (wTar.pitch    - wCur.pitch)    * alpha;
  wCur.roll     += (wTar.roll     - wCur.roll)     * alpha;
  wCur.yaw      += (wTar.yaw      - wCur.yaw)      * alpha;
  wCur.palmArch += (wTar.palmArch - wCur.palmArch) * alpha;

  if (wristGimbal) {
    wristGimbal.rotation.x = wCur.pitch;
    wristGimbal.rotation.y = wCur.yaw;
    wristGimbal.rotation.z = wCur.roll;
  }

  if (rightPalmArchGroup) {
    rightPalmArchGroup.rotation.y = -wCur.palmArch * 0.45;
    rightPalmArchGroup.rotation.z = -wCur.palmArch * 0.25;
  }

  updateWristTelemetryUI(wCur);

  // 2. Interpolate All 5 Fingers
  FINGER_CONFIGS.forEach(cfg => {
    const fState = handState.fingers[cfg.id];
    const cur = fState.current;
    const tar = fState.target;

    cur.mcp   += (tar.mcp   - cur.mcp)   * alpha;
    cur.pip   += (tar.pip   - cur.pip)   * alpha;
    cur.dip   += (tar.dip   - cur.dip)   * alpha;
    cur.splay += (tar.splay - cur.splay) * alpha;

    const rig = fingerRigNodes[cfg.id];
    if (rig) {
      if (cfg.isThumb) {
        // Biomechanical Thumb Opposition: coupled inward convergence across the palm
        rig.splayGroup.rotation.z = cur.splay;
        rig.splayGroup.rotation.y = cur.mcp * 0.45;
        rig.mcpJoint.rotation.x = cur.mcp * 0.85;
        rig.mcpJoint.rotation.z = -cur.mcp * 0.42;
        rig.pipJoint.rotation.x = cur.pip * 0.95;
        rig.dipJoint.rotation.x = cur.dip * 1.05;
      } else {
        rig.splayGroup.rotation.z = cur.splay;
        rig.mcpJoint.rotation.x = cur.mcp;
        rig.pipJoint.rotation.x = cur.pip;
        rig.dipJoint.rotation.x = cur.dip;
      }
    }

    updateFingerTelemetryUI(cfg.id, cur);
  });
}
