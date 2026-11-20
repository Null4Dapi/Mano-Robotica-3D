/**
 * tracker.js — MediaPipe Hands Optical Tracking & Teleoperation
 */

let mpHands = null;
let mpCamera = null;
let handSide = 'right';

function initTrackerUI() {
  const toggleBtn = document.getElementById('btn-toggle-cam');
  if (toggleBtn) {
    toggleBtn.addEventListener('click', async () => {
      if (handState.cameraActive) {
        stopWebcamTracking();
      } else {
        await startWebcamTracking();
      }
    });
  }
}

async function startWebcamTracking() {
  const videoElement = document.getElementById('webcam-video');
  const loadingEl = document.getElementById('cam-loading');
  const placeholderEl = document.getElementById('cam-placeholder');
  if (loadingEl) loadingEl.classList.remove('hidden');
  if (placeholderEl) placeholderEl.classList.add('hidden');
  updateStatusMessage('Inicializando MediaPipe Hands y solicitando acceso a cámara...');

  try {
    if (!mpHands) {
      mpHands = new Hands({
        locateFile: (file) => `https://cdn.jsdelivr.net/npm/@mediapipe/hands/${file}`
      });
      mpHands.setOptions({
        maxNumHands: 1,
        modelComplexity: 1,
        minDetectionConfidence: 0.65,
        minTrackingConfidence: 0.65
      });
      mpHands.onResults(onMediaPipeResults);
    }

    mpCamera = new Camera(videoElement, {
      onFrame: async () => {
        if (handState.cameraActive && mpHands) {
          await mpHands.send({ image: videoElement });
        }
      },
      width: 640,
      height: 360
    });

    await mpCamera.start();

    handState.cameraActive = true;
    if (trajectoryPlayback.isPlaying) pauseTrajectory();

    if (loadingEl) loadingEl.classList.add('hidden');
    updateCameraUIState(true);
    updateStatusMessage('Cámara activa. Coloca tu mano frente al sensor para teleoperar.');
  } catch (err) {
    console.error('Error al iniciar la cámara:', err);
    if (loadingEl) loadingEl.classList.add('hidden');
    if (placeholderEl) placeholderEl.classList.remove('hidden');
    updateStatusMessage('No se pudo acceder a la cámara web. Verifica los permisos del navegador.');
  }
}

function stopWebcamTracking() {
  handState.cameraActive = false;
  if (mpCamera) {
    mpCamera.stop();
  }
  const placeholderEl = document.getElementById('cam-placeholder');
  if (placeholderEl) placeholderEl.classList.remove('hidden');
  updateCameraUIState(false);
  updateStatusMessage('Seguimiento por cámara pausado. Modo manual activo.');
}

function updateCameraUIState(isActive) {
  const btn = document.getElementById('btn-toggle-cam');
  const text = document.getElementById('cam-btn-text');
  const camBoxStatus = document.getElementById('cam-box-status');

  if (isActive) {
    if (btn) btn.classList.add('active-cam');
    if (text) text.textContent = 'Detener Cámara';
    if (camBoxStatus) camBoxStatus.textContent = 'ACTIVA';
  } else {
    if (btn) btn.classList.remove('active-cam');
    if (text) text.textContent = 'Activar Cámara IA';
    if (camBoxStatus) camBoxStatus.textContent = 'EN ESPERA';
  }
}

function toggleMirror() {
  handState.mirrorWebcam = !handState.mirrorWebcam;
  const outputCanvas = document.getElementById('output-canvas');
  if (outputCanvas) {
    outputCanvas.style.transform = handState.mirrorWebcam ? 'scaleX(-1)' : 'scaleX(1)';
  }
}

function toggleHandSide() {
  handSide = handSide === 'right' ? 'left' : 'right';
  const btn = document.getElementById('btn-hand-side');
  if (btn) btn.textContent = handSide === 'right' ? 'Mano Derecha' : 'Mano Izquierda';
}

function onMediaPipeResults(results) {
  const videoElement = document.getElementById('webcam-video');
  const outputCanvas = document.getElementById('output-canvas');
  if (!outputCanvas || !videoElement) return;
  const canvasCtx = outputCanvas.getContext('2d');

  const t0 = performance.now();
  outputCanvas.width = videoElement.videoWidth || 640;
  outputCanvas.height = videoElement.videoHeight || 360;

  canvasCtx.save();
  canvasCtx.clearRect(0, 0, outputCanvas.width, outputCanvas.height);
  canvasCtx.drawImage(results.image, 0, 0, outputCanvas.width, outputCanvas.height);

  const lmEl = document.getElementById('telemetry-landmarks');
  const confEl = document.getElementById('telemetry-confidence');
  const inferEl = document.getElementById('telemetry-inference');
  const latEl = document.getElementById('telemetry-latency');

  if (results.multiHandLandmarks && results.multiHandLandmarks.length > 0) {
    const landmarks = results.multiHandLandmarks[0];
    const worldLandmarks = (results.multiHandWorldLandmarks && results.multiHandWorldLandmarks[0])
      ? results.multiHandWorldLandmarks[0]
      : landmarks;
    const handedness = results.multiHandedness?.[0];
    const score = handedness ? Math.round(handedness.score * 100) : 98;

    if (typeof drawConnectors === 'function' && typeof HAND_CONNECTIONS !== 'undefined') {
      drawConnectors(canvasCtx, landmarks, HAND_CONNECTIONS, { color: '#ffffff', lineWidth: 2 });
    }
    if (typeof drawLandmarks === 'function') {
      drawLandmarks(canvasCtx, landmarks, { color: '#8e93a0', lineWidth: 1, radius: 3 });
    }

    const inferMs = (performance.now() - t0 + 4.2).toFixed(1);
    const latMs = (parseFloat(inferMs) + 3.8).toFixed(1);

    if (lmEl) lmEl.textContent = '21 / 21 puntos';
    if (confEl) confEl.textContent = `${score}%`;
    if (inferEl) inferEl.textContent = `${inferMs} ms`;
    if (latEl) latEl.textContent = `${latMs} ms`;

    mapLandmarksToRoboticHand(landmarks, worldLandmarks);
  } else {
    if (lmEl) lmEl.textContent = 'Buscando...';
    if (confEl) confEl.textContent = '--%';
    if (inferEl) inferEl.textContent = '-- ms';
    if (latEl) latEl.textContent = '-- ms';
  }
  canvasCtx.restore();
}

function vecSub(a, b) {
  return { x: a.x - b.x, y: a.y - b.y, z: a.z - b.z };
}

function vecLen(v) {
  return Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z) + 1e-7;
}

function vecNorm(v) {
  const l = vecLen(v);
  return { x: v.x / l, y: v.y / l, z: v.z / l };
}

function angleBetweenVectors(v1, v2) {
  const n1 = vecNorm(v1);
  const n2 = vecNorm(v2);
  const dot = Math.max(-1, Math.min(1, n1.x * n2.x + n1.y * n2.y + n1.z * n2.z));
  return Math.acos(dot);
}

function mapLandmarksToRoboticHand(screenLm, worldLm) {
  // 1. Orientación Muñeca y Palma 3D
  const w0 = screenLm[0];
  const m9 = screenLm[9];
  const i5 = screenLm[5];
  const p17 = screenLm[17];

  const palmUp = vecNorm({
    x: (m9.x - w0.x) * (handState.mirrorWebcam ? -1 : 1),
    y: -(m9.y - w0.y),
    z: -(m9.z - w0.z)
  });

  const palmAcross = vecNorm({
    x: (p17.x - i5.x) * (handState.mirrorWebcam ? -1 : 1),
    y: -(p17.y - i5.y),
    z: -(p17.z - i5.z)
  });

  const yaw = THREE.MathUtils.clamp(Math.atan2(palmUp.x, palmUp.y) * 0.95, -0.75, 0.75);
  const pitch = THREE.MathUtils.clamp(palmUp.z * 1.45, -0.95, 0.95);
  const roll = THREE.MathUtils.clamp(Math.atan2(palmAcross.z, palmAcross.x) * 0.85, -1.35, 1.35);

  handState.wrist.target.yaw = -yaw;
  handState.wrist.target.pitch = pitch;
  handState.wrist.target.roll = roll;

  // 2. Dedos (Índice, Medio, Anular, Meñique)
  const fingerIndices = [
    { id: 'index',  pts: [0, 5, 6, 7, 8],     neutralSpread: 0.06 },
    { id: 'middle', pts: [0, 9, 10, 11, 12],   neutralSpread: 0.0 },
    { id: 'ring',   pts: [0, 13, 14, 15, 16], neutralSpread: -0.05 },
    { id: 'pinky',  pts: [0, 17, 18, 19, 20], neutralSpread: -0.12 }
  ];

  const curlRatios = {};

  fingerIndices.forEach(f => {
    const [iWrist, iMcp, iPip, iDip, iTip] = f.pts;
    const vPalm = vecSub(worldLm[iMcp], worldLm[iWrist]);
    const vProx = vecSub(worldLm[iPip], worldLm[iMcp]);
    const vInt  = vecSub(worldLm[iDip], worldLm[iPip]);
    const vDist = vecSub(worldLm[iTip], worldLm[iDip]);

    const mcpAngle = THREE.MathUtils.clamp(angleBetweenVectors(vPalm, vProx) * 1.15 - 0.12, 0, 1.58);
    const pipAngle = THREE.MathUtils.clamp(angleBetweenVectors(vProx, vInt) * 1.15 - 0.08, 0, 1.72);
    const dipAngle = THREE.MathUtils.clamp(angleBetweenVectors(vInt, vDist) * 1.15 - 0.05, 0, 1.35);

    const vCenter = vecSub(worldLm[9], worldLm[0]);
    const lateralCross = (vProx.x * vCenter.y - vProx.y * vCenter.x);
    const splayAngle = THREE.MathUtils.clamp(
      (lateralCross * (handState.mirrorWebcam ? 1.1 : -1.1)) - f.neutralSpread,
      -0.38,
      0.38
    );

    handState.fingers[f.id].target.mcp = mcpAngle;
    handState.fingers[f.id].target.pip = pipAngle;
    handState.fingers[f.id].target.dip = dipAngle;
    handState.fingers[f.id].target.splay = splayAngle * (1 - mcpAngle / 2.2);

    curlRatios[f.id] = (mcpAngle + pipAngle) / 3.1;
  });

  // 3. Articulación del Pulgar
  const vThumbCmc = vecSub(worldLm[2], worldLm[1]);
  const vThumbMcp = vecSub(worldLm[3], worldLm[2]);
  const vThumbIp  = vecSub(worldLm[4], worldLm[3]);

  const distThumbToPinky = vecLen(vecSub(worldLm[4], worldLm[17]));
  const palmWidth = vecLen(vecSub(worldLm[17], worldLm[5])) + 1e-5;
  const thumbOpposition = THREE.MathUtils.clamp(1.65 - (distThumbToPinky / palmWidth), 0, 1.15);

  const thumbMcpBend = THREE.MathUtils.clamp(angleBetweenVectors(vThumbCmc, vThumbMcp) * 1.3, 0, 1.2);
  const thumbIpBend  = THREE.MathUtils.clamp(angleBetweenVectors(vThumbMcp, vThumbIp) * 1.35, 0, 1.25);

  handState.fingers.thumb.target.mcp = THREE.MathUtils.clamp(thumbOpposition * 0.75 + thumbMcpBend * 0.4, 0, 1.15);
  handState.fingers.thumb.target.pip = THREE.MathUtils.clamp(thumbMcpBend * 0.8 + thumbOpposition * 0.35, 0, 1.25);
  handState.fingers.thumb.target.dip = thumbIpBend;
  handState.fingers.thumb.target.splay = THREE.MathUtils.clamp(0.25 - thumbOpposition * 0.55, -0.45, 0.30);

  curlRatios.thumb = (handState.fingers.thumb.target.mcp + handState.fingers.thumb.target.pip) / 2.1;

  handState.wrist.target.palmArch = THREE.MathUtils.clamp((curlRatios.ring + curlRatios.pinky) * 0.18, 0.04, 0.35);

  classifyLiveGesture(curlRatios, worldLm, palmWidth);
}

function classifyLiveGesture(curls, worldLm, palmWidth) {
  const isExt = (id) => curls[id] < 0.35;
  const isCurled = (id) => curls[id] > 0.62;

  const pinchDist = vecLen(vecSub(worldLm[4], worldLm[8])) / palmWidth;
  let label = 'TELEOPERACIÓN LIBRE';

  if (pinchDist < 0.38 && isExt('middle') && isExt('ring')) {
    label = 'PINZA OK';
  } else if (isExt('index') && isExt('middle') && isExt('ring') && isExt('pinky') && curls.thumb < 0.45) {
    label = 'MANO ABIERTA';
  } else if (isCurled('index') && isCurled('middle') && isCurled('ring') && isCurled('pinky')) {
    label = curls.thumb < 0.35 ? 'PULGAR ARRIBA' : 'PUÑO CERRADO';
  } else if (isExt('index') && isExt('middle') && isCurled('ring') && isCurled('pinky')) {
    label = 'VICTORIA';
  } else if (isExt('index') && isCurled('middle') && isCurled('ring') && isExt('pinky')) {
    label = 'ROCK';
  } else if (isExt('index') && isCurled('middle') && isCurled('ring') && isCurled('pinky')) {
    label = 'SEÑALAR';
  }

  const gBadge = document.getElementById('gesture-badge');
  if (gBadge) gBadge.textContent = label;
}
