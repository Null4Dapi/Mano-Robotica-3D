/**
 * app.js — Main Application Bootstrap & Animation Loop
 */

function animate(now) {
  requestAnimationFrame(animate);
  applyHandKinematics();
  if (renderer && scene && camera) {
    renderer.render(scene, camera);
  }
}

window.addEventListener('DOMContentLoaded', () => {
  buildServoSlidersUI();
  initThreeScene();
  initTrackerUI();
  applyPreset('open');
  fetchTrajectoryJson(false);
  animate(performance.now());
});
