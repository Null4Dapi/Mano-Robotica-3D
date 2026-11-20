/**
 * hand_model.js — Three.js 3D Hand Scene, Bionic Meshes & Camera Controls
 */

    let scene, camera, renderer;
    let handRoot, wristGimbal, palmGroup, rightPalmArchGroup;
    const fingerRigNodes = {};

    const camOrbit = {
      isDragging: false,
      prevX: 0,
      prevY: 0,
      theta: 0.35,
      phi: 1.35,
      radius: 10.5,
      targetY: 1.3
    };

    function initThreeScene() {
      const container = document.getElementById('canvas-container');
      const canvas = document.getElementById('three-canvas');

      scene = new THREE.Scene();
      scene.background = new THREE.Color(0x0a0b0e);
      scene.fog = new THREE.FogExp2(0x0a0b0e, 0.035);

      camera = new THREE.PerspectiveCamera(42, container.clientWidth / container.clientHeight, 0.1, 100);
      updateCameraOrbitPosition();

      renderer = new THREE.WebGLRenderer({ canvas, antialias: true, powerPreference: 'high-performance' });
      renderer.setSize(container.clientWidth, container.clientHeight);
      renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
      renderer.shadowMap.enabled = true;
      renderer.shadowMap.type = THREE.PCFSoftShadowMap;
      renderer.toneMapping = THREE.ACESFilmicToneMapping;
      renderer.toneMappingExposure = 1.15;

      // Clean cool monochromatic lighting
      const ambientLight = new THREE.AmbientLight(0x71717a, 0.75);
      scene.add(ambientLight);

      const mainDirLight = new THREE.DirectionalLight(0xffffff, 1.4);
      mainDirLight.position.set(6, 12, 9);
      mainDirLight.castShadow = true;
      mainDirLight.shadow.mapSize.width = 2048;
      mainDirLight.shadow.mapSize.height = 2048;
      scene.add(mainDirLight);

      const rimLight1 = new THREE.PointLight(0xe4e4e7, 0.75, 24);
      rimLight1.position.set(-7, 6, -5);
      scene.add(rimLight1);

      const rimLight2 = new THREE.PointLight(0x8e93a0, 0.55, 20);
      rimLight2.position.set(7, -3, 6);
      scene.add(rimLight2);

      // Floor Grid: subtle dark gray (0x3f3f46, 0x18181b)
      const gridHelper = new THREE.GridHelper(28, 28, 0x3f3f46, 0x18181b);
      gridHelper.position.y = -3.4;
      gridHelper.material.opacity = 0.5;
      gridHelper.material.transparent = true;
      scene.add(gridHelper);

      // Floor Base Ring
      const ringGeo = new THREE.RingGeometry(1.7, 1.85, 64);
      const ringMat = new THREE.MeshBasicMaterial({ color: 0x282b34, side: THREE.DoubleSide, transparent: true, opacity: 0.5 });
      const baseRing = new THREE.Mesh(ringGeo, ringMat);
      baseRing.rotation.x = Math.PI / 2;
      baseRing.position.y = -3.38;
      scene.add(baseRing);

      createRobotMaterials();
      buildRoboticHand();
      setupViewportInteractions(canvas);

      window.addEventListener('resize', onWindowResize);
    }

    function onWindowResize() {
      const container = document.getElementById('canvas-container');
      if (!container || !renderer || !camera) return;
      camera.aspect = container.clientWidth / container.clientHeight;
      camera.updateProjectionMatrix();
      renderer.setSize(container.clientWidth, container.clientHeight);
    }


    function buildRoboticHand() {
      handRoot = new THREE.Group();
      scene.add(handRoot);

      // =========================================================================
      // 1. FOREARM PYLON & DUAL LINEAR ACTUATION RIG
      // =========================================================================
      const forearmGroup = new THREE.Group();
      forearmGroup.position.set(0, -1.95, 0);
      handRoot.add(forearmGroup);

      // Main structural pylon: octagonal tapered prism with hollowed inner channel
      const pylonGeo = new THREE.CylinderGeometry(0.88, 1.15, 2.4, 8, 1);
      const forearmPylon = new THREE.Mesh(pylonGeo, materials.darkFrame);
      forearmPylon.castShadow = true;
      forearmPylon.receiveShadow = true;
      forearmGroup.add(forearmPylon);

      // Carbon-fiber exoskeleton truss plates (left & right flank braces)
      [-0.82, 0.82].forEach(xSign => {
        const braceGeo = new THREE.BoxGeometry(0.12, 2.1, 0.72);
        const brace = new THREE.Mesh(braceGeo, materials.armor);
        brace.position.set(xSign, -0.05, 0);
        brace.castShadow = true;
        forearmGroup.add(brace);

        // Allen head mounting bolts on the flank braces
        [-0.7, 0, 0.7].forEach(yOffset => {
          const boltGeo = new THREE.CylinderGeometry(0.06, 0.06, 0.16, 6);
          const bolt = new THREE.Mesh(boltGeo, materials.fastenerAlloy);
          bolt.rotation.z = Math.PI / 2;
          bolt.position.set(xSign * 1.05, yOffset, 0);
          forearmGroup.add(bolt);
        });
      });

      // Dual hydraulic/electromechanical pushrod actuators (connecting to wrist)
      [-0.55, 0.55].forEach(xPos => {
        // Outer cylinder barrel
        const barrelGeo = new THREE.CylinderGeometry(0.16, 0.18, 1.6, 16);
        const barrel = new THREE.Mesh(barrelGeo, materials.armor);
        barrel.position.set(xPos, 0.1, 0.52);
        barrel.castShadow = true;
        forearmGroup.add(barrel);

        // Bronze sealing gland collar
        const collarGeo = new THREE.CylinderGeometry(0.19, 0.19, 0.1, 16);
        const collar = new THREE.Mesh(collarGeo, materials.bronzeBushing);
        collar.position.set(xPos, 0.85, 0.52);
        forearmGroup.add(collar);

        // Inner polished chrome ram extending upward toward wrist gimbal
        const ramGeo = new THREE.CylinderGeometry(0.09, 0.09, 1.1, 16);
        const ram = new THREE.Mesh(ramGeo, materials.jointChrome);
        ram.position.set(xPos, 1.15, 0.52);
        ram.castShadow = true;
        forearmGroup.add(ram);

        // Spherical rod-end bearing (clevis)
        const rodEndGeo = new THREE.SphereGeometry(0.12, 12, 12);
        const rodEnd = new THREE.Mesh(rodEndGeo, materials.jointChrome);
        rodEnd.position.set(xPos, 1.68, 0.52);
        forearmGroup.add(rodEnd);
      });

      // Braided cable conduit sleeve along dorsal center
      const conduitGeo = new THREE.CylinderGeometry(0.18, 0.22, 2.2, 16);
      const conduit = new THREE.Mesh(conduitGeo, materials.sensorPad);
      conduit.position.set(0, 0.1, -0.48);
      forearmGroup.add(conduit);

      // Wrist Mounting Ring & Optical Encoder Flange
      const mountFlangeGeo = new THREE.CylinderGeometry(1.02, 0.94, 0.24, 24);
      const mountFlange = new THREE.Mesh(mountFlangeGeo, materials.armor);
      mountFlange.position.y = 1.08;
      mountFlange.castShadow = true;
      forearmGroup.add(mountFlange);

      // Segmented status optical ring (4 discrete quadrant indicators)
      for (let i = 0; i < 4; i++) {
        const segGeo = new THREE.TorusGeometry(0.96, 0.032, 8, 16, Math.PI * 0.38);
        const seg = new THREE.Mesh(segGeo, materials.ledGlow);
        seg.rotation.x = Math.PI / 2;
        seg.rotation.z = (Math.PI / 2) * i + Math.PI * 0.06;
        seg.position.y = 1.18;
        forearmGroup.add(seg);
      }

      // =========================================================================
      // 2. INDUSTRIAL BIAXIAL GIMBAL WRIST (CARDAN YOKE)
      // =========================================================================
      wristGimbal = new THREE.Group();
      wristGimbal.position.set(0, -0.42, 0);
      handRoot.add(wristGimbal);

      // Outer Cardan U-Yoke (machined titanium fork)
      const yokeBaseGeo = new THREE.CylinderGeometry(0.58, 0.62, 0.32, 18);
      const yokeBase = new THREE.Mesh(yokeBaseGeo, materials.armor);
      yokeBase.position.y = -0.16;
      wristGimbal.add(yokeBase);

      // Twin upright yoke arms
      [-0.72, 0.72].forEach(xSign => {
        const armGeo = new THREE.BoxGeometry(0.24, 0.65, 0.52);
        const arm = new THREE.Mesh(armGeo, materials.armor);
        arm.position.set(xSign, 0.22, 0);
        arm.castShadow = true;
        wristGimbal.add(arm);

        // Precision bronze bushing sleeve on each arm
        const bushingGeo = new THREE.CylinderGeometry(0.26, 0.26, 0.26, 16);
        const bushing = new THREE.Mesh(bushingGeo, materials.bronzeBushing);
        bushing.rotation.z = Math.PI / 2;
        bushing.position.set(xSign, 0.35, 0);
        wristGimbal.add(bushing);

        // Allen cap screw retainer
        const capGeo = new THREE.CylinderGeometry(0.18, 0.18, 0.32, 6);
        const cap = new THREE.Mesh(capGeo, materials.fastenerAlloy);
        cap.rotation.z = Math.PI / 2;
        cap.position.set(xSign * 1.08, 0.35, 0);
        wristGimbal.add(cap);
      });

      // Central Cardan Cross-Journal Pin (hard chrome steel)
      const crossPinGeo = new THREE.CylinderGeometry(0.22, 0.22, 1.48, 18);
      const crossPin = new THREE.Mesh(crossPinGeo, materials.jointChrome);
      crossPin.rotation.z = Math.PI / 2;
      crossPin.position.y = 0.35;
      crossPin.castShadow = true;
      wristGimbal.add(crossPin);

      // Spherical central core enclosing the cardan intersection
      const gimbalCore = new THREE.Mesh(
        new THREE.SphereGeometry(0.48, 20, 20),
        materials.darkFrame
      );
      gimbalCore.position.y = 0.35;
      wristGimbal.add(gimbalCore);

      // =========================================================================
      // 3. ANATOMICAL METACARPAL PALM CHASSIS
      // =========================================================================
      palmGroup = new THREE.Group();
      palmGroup.position.set(0, 0.35, 0);
      wristGimbal.add(palmGroup);

      // Sculpted Carpal-Metacarpal Chassis: Tapered anatomical bridge
      const palmCoreGeo = new THREE.BoxGeometry(2.18, 1.95, 0.58);
      const palmCore = new THREE.Mesh(palmCoreGeo, materials.darkFrame);
      palmCore.position.set(-0.06, 1.02, 0);
      palmCore.castShadow = true;
      palmCore.receiveShadow = true;
      palmGroup.add(palmCore);

      // Dorsal Exoskeleton Carapace: Sculpted titanium shell with distinct metacarpal channels
      const dorsalShellGeo = new THREE.BoxGeometry(2.12, 1.88, 0.18);
      const dorsalShell = new THREE.Mesh(dorsalShellGeo, materials.armor);
      dorsalShell.position.set(-0.06, 1.04, -0.28);
      dorsalShell.castShadow = true;
      palmGroup.add(dorsalShell);

      // 4 Metacarpal Structural Spine Ridges (Rayos metacarpianos II al V)
      [-0.82, -0.25, 0.35, 0.92].forEach((xPos, idx) => {
        const ridgeH = 1.65 - Math.abs(xPos) * 0.15;
        const ridgeGeo = new THREE.CylinderGeometry(0.08, 0.11, ridgeH, 8);
        const ridge = new THREE.Mesh(ridgeGeo, materials.jointChrome);
        ridge.position.set(xPos, 1.0 + (idx === 1 ? 0.08 : 0), -0.36);
        ridge.castShadow = true;
        palmGroup.add(ridge);
      });

      // Recessed Central Telemetry Matrix (Status window on the dorsal plate)
      const telemetryPlaque = new THREE.Mesh(
        new THREE.BoxGeometry(0.85, 0.65, 0.04),
        materials.fastenerAlloy
      );
      telemetryPlaque.position.set(-0.06, 0.95, -0.38);
      palmGroup.add(telemetryPlaque);

      const statusGlowStrip = new THREE.Mesh(
        new THREE.BoxGeometry(0.72, 0.06, 0.05),
        materials.ledGlow
      );
      statusGlowStrip.position.set(-0.06, 1.15, -0.39);
      palmGroup.add(statusGlowStrip);

      // PALMAR (VOLAR) FACE: Ergonomic High-Friction Elastomer Grip Architecture
      // 1) Distal transverse metacarpal ridge pad (treaded strip under knuckles)
      const palmarTreadGeo = new THREE.BoxGeometry(1.85, 0.45, 0.14);
      const palmarTread = new THREE.Mesh(palmarTreadGeo, materials.sensorPad);
      palmarTread.position.set(-0.05, 1.72, 0.34);
      palmGroup.add(palmarTread);

      // Micro-ribs for palmar grip
      [-0.12, 0, 0.12].forEach(yOff => {
        const ribGeo = new THREE.BoxGeometry(1.75, 0.04, 0.04);
        const rib = new THREE.Mesh(ribGeo, materials.armor);
        rib.position.set(-0.05, 1.72 + yOff, 0.41);
        palmGroup.add(rib);
      });

      // 2) Thenar Eminence Cushion (almohadilla tenar en la base medial)
      const thenarCushionGeo = new THREE.SphereGeometry(0.46, 16, 16);
      const thenarCushion = new THREE.Mesh(thenarCushionGeo, materials.sensorPad);
      thenarCushion.scale.set(0.88, 1.30, 0.58);
      thenarCushion.position.set(-0.66, 0.74, 0.30);
      palmGroup.add(thenarCushion);

      // 3) Thenar Mechanical Saddle Bracket (Base articular CMC de aleación de titanio)
      const thenarBracketGeo = new THREE.BoxGeometry(0.58, 0.70, 0.54);
      const thenarBracket = new THREE.Mesh(thenarBracketGeo, materials.armor);
      thenarBracket.position.set(-0.88, 0.60, 0.16);
      thenarBracket.rotation.y = 0.28;
      thenarBracket.castShadow = true;
      palmGroup.add(thenarBracket);

      // Bronze socket ring for thumb ball swivel
      const thenarSocketRing = new THREE.Mesh(
        new THREE.TorusGeometry(0.26, 0.055, 8, 20),
        materials.bronzeBushing
      );
      thenarSocketRing.position.set(-0.92, 0.58, 0.20);
      thenarSocketRing.rotation.x = Math.PI / 4;
      palmGroup.add(thenarSocketRing);

      // =========================================================================
      // HYPOTHENAR CUPPING ARCH (ULNAR 4th & 5th METACARPAL RIG)
      // =========================================================================
      rightPalmArchGroup = new THREE.Group();
      rightPalmArchGroup.position.set(0.38, 0.0, 0.0);
      palmGroup.add(rightPalmArchGroup);

      // Ulnar palmar plate (enables hand cupping around cups/spheres)
      const ulnarPlateGeo = new THREE.BoxGeometry(0.88, 1.88, 0.54);
      const ulnarPlate = new THREE.Mesh(ulnarPlateGeo, materials.armor);
      ulnarPlate.position.set(0.48, 1.02, 0.02);
      ulnarPlate.castShadow = true;
      rightPalmArchGroup.add(ulnarPlate);

      // Hypothenar elastomer pad (almohadilla hipotenar ulnar)
      const hypothenarCushionGeo = new THREE.BoxGeometry(0.68, 1.15, 0.12);
      const hypothenarCushion = new THREE.Mesh(hypothenarCushionGeo, materials.sensorPad);
      hypothenarCushion.position.set(0.52, 0.85, 0.32);
      rightPalmArchGroup.add(hypothenarCushion);

      // =========================================================================
      // 4. ASSEMBLE ALL 5 ARTICULATED FINGERS (18 GDL TOTAL)
      // =========================================================================
      FINGER_CONFIGS.forEach(cfg => {
        const fingerRig = createArticulatedFinger(cfg);
        fingerRigNodes[cfg.id] = fingerRig;

        if (cfg.id === 'ring' || cfg.id === 'pinky') {
          // Compensate for ulnar arch pivot offset
          fingerRig.root.position.x -= 0.38;
          rightPalmArchGroup.add(fingerRig.root);
        } else {
          palmGroup.add(fingerRig.root);
        }
      });
    }

    function createArticulatedFinger(cfg) {
      const root = new THREE.Group();
      root.position.set(cfg.basePos.x, cfg.basePos.y, cfg.basePos.z);
      root.rotation.set(cfg.baseRot.x, cfg.baseRot.y, cfg.baseRot.z);

      // Lateral splay (abduction / adduction) group
      const splayGroup = new THREE.Group();
      root.add(splayGroup);

      // Metacarpophalangeal Joint (MCP)
      const mcpJoint = new THREE.Group();
      splayGroup.add(mcpJoint);
      addKnuckleServoMesh(mcpJoint, cfg.radii[0] * 1.06, true, cfg.isThumb);

      // Proximal Phalanx
      const len1 = cfg.lengths[0];
      const r1 = cfg.radii[0];
      addPhalanxSegmentMesh(mcpJoint, len1, r1, r1 * 0.92, 0, cfg.isThumb);

      // Proximal Interphalangeal Joint (PIP)
      const pipJoint = new THREE.Group();
      pipJoint.position.set(0, len1, 0);
      mcpJoint.add(pipJoint);
      addKnuckleServoMesh(pipJoint, cfg.radii[1] * 1.02, false, cfg.isThumb);

      // Intermediate Phalanx
      const len2 = cfg.lengths[1];
      const r2 = cfg.radii[1];
      addPhalanxSegmentMesh(pipJoint, len2, r2, r2 * 0.88, 1, cfg.isThumb);

      // Distal Interphalangeal Joint (DIP)
      const dipJoint = new THREE.Group();
      dipJoint.position.set(0, len2, 0);
      pipJoint.add(dipJoint);
      addKnuckleServoMesh(dipJoint, cfg.radii[2] * 0.96, false, cfg.isThumb);

      // Distal Phalanx & Bio-Inspired Fingertip Pulp
      const len3 = cfg.lengths[2];
      const r3 = cfg.radii[2];
      addPhalanxSegmentMesh(dipJoint, len3 * 0.76, r3, r3 * 0.82, 2, cfg.isThumb);

      // Anatomical High-Friction Tactile Fingertip (Pulpejo Ergonómico)
      const pulpGeo = new THREE.SphereGeometry(r3 * 0.88, 18, 18);
      const pulpMesh = new THREE.Mesh(pulpGeo, materials.sensorPad);
      pulpMesh.scale.set(0.96, 1.35, 1.08);
      pulpMesh.position.set(0, len3 * 0.78, cfg.isThumb ? r3 * 0.22 : r3 * 0.18);
      pulpMesh.castShadow = true;
      dipJoint.add(pulpMesh);

      // Protective Dorsal Nail Shell
      const nailGeo = new THREE.CylinderGeometry(r3 * 0.75, r3 * 0.82, len3 * 0.42, 14, 1, false, Math.PI, Math.PI);
      const nailMesh = new THREE.Mesh(nailGeo, materials.armor);
      nailMesh.position.set(0, len3 * 0.76, -r3 * 0.12);
      nailMesh.castShadow = true;
      dipJoint.add(nailMesh);

      // Optical Proximity / Contact Force Sensor Aperture at tip
      const tipSensorGeo = new THREE.CylinderGeometry(r3 * 0.32, r3 * 0.32, 0.05, 16);
      const tipSensor = new THREE.Mesh(tipSensorGeo, materials.ledGlow);
      tipSensor.position.set(0, len3 * 0.98, 0);
      dipJoint.add(tipSensor);

      return { root, splayGroup, mcpJoint, pipJoint, dipJoint };
    }

    function addKnuckleServoMesh(jointGroup, radius, hasLedRing, isThumb = false) {
      if (isThumb) {
        // True spherical low-profile ball-clevis for thumb CMC/MCP opposition
        const ballCore = new THREE.Mesh(
          new THREE.SphereGeometry(radius * 1.05, 20, 20),
          materials.jointChrome
        );
        ballCore.castShadow = true;
        jointGroup.add(ballCore);

        // Surrounding bronze retention collar
        const collarRing = new THREE.Mesh(
          new THREE.TorusGeometry(radius * 0.98, 0.055, 10, 24),
          materials.bronzeBushing
        );
        collarRing.rotation.x = Math.PI / 2;
        jointGroup.add(collarRing);

        if (hasLedRing) {
          const telemetryRing = new THREE.Mesh(
            new THREE.TorusGeometry(radius * 0.88, 0.024, 8, 20),
            materials.ledGlow
          );
          jointGroup.add(telemetryRing);
        }
        return;
      }

      // High-precision bifurcated knuckle hinge for fingers
      // 1. Central Hard Chrome Bearing Axle Pin
      const axleLength = radius * 1.88;
      const axleGeo = new THREE.CylinderGeometry(radius * 0.58, radius * 0.58, axleLength, 18);
      const axlePin = new THREE.Mesh(axleGeo, materials.jointChrome);
      axlePin.rotation.z = Math.PI / 2;
      axlePin.castShadow = true;
      jointGroup.add(axlePin);

      // 2. Dual Sintered Phosphor Bronze Bushing Rings (flank washers)
      [-axleLength * 0.42, axleLength * 0.42].forEach(xSign => {
        const bushingGeo = new THREE.CylinderGeometry(radius * 0.84, radius * 0.84, radius * 0.18, 16);
        const bushing = new THREE.Mesh(bushingGeo, materials.bronzeBushing);
        bushing.rotation.z = Math.PI / 2;
        bushing.position.x = xSign;
        jointGroup.add(bushing);

        // Allen cap screw hardware detail on outer ends
        const boltGeo = new THREE.CylinderGeometry(radius * 0.42, radius * 0.42, radius * 0.12, 6);
        const bolt = new THREE.Mesh(boltGeo, materials.fastenerAlloy);
        bolt.rotation.z = Math.PI / 2;
        bolt.position.x = xSign * 1.15;
        jointGroup.add(bolt);
      });

      // 3. Central Pivot Drum / Hinge Barrel
      const drumGeo = new THREE.CylinderGeometry(radius * 0.92, radius * 0.92, radius * 0.72, 18);
      const drum = new THREE.Mesh(drumGeo, materials.armor);
      drum.rotation.z = Math.PI / 2;
      drum.castShadow = true;
      jointGroup.add(drum);

      // 4. Integrated Mechanical Extension-Stop Cowl (anti-hyperextension barrier)
      const stopCowlGeo = new THREE.BoxGeometry(radius * 1.25, radius * 0.35, radius * 0.82);
      const stopCowl = new THREE.Mesh(stopCowlGeo, materials.darkFrame);
      stopCowl.position.set(0, -radius * 0.42, -radius * 0.52);
      jointGroup.add(stopCowl);

      // 5. Optical joint encoder ring on MCP knuckles
      if (hasLedRing) {
        const ringGeo = new THREE.CylinderGeometry(radius * 0.96, radius * 0.96, 0.05, 18);
        const ring = new THREE.Mesh(ringGeo, materials.ledGlow);
        ring.rotation.z = Math.PI / 2;
        jointGroup.add(ring);
      }
    }

    function addPhalanxSegmentMesh(parentJoint, length, bottomRadius, topRadius, phalanxIndex = 0, isThumb = false) {
      const midY = length / 2;

      // 1. Central Structural Core: Lightweight carbon-fiber structural shaft
      const coreGeo = new THREE.CylinderGeometry(topRadius * 0.70, bottomRadius * 0.74, length * 0.92, 14);
      const innerCore = new THREE.Mesh(coreGeo, materials.darkFrame);
      innerCore.position.set(0, midY, 0);
      innerCore.castShadow = true;
      parentJoint.add(innerCore);

      // 2. Dual Titanium Side Flank Plates (Skeletonized Truss Exoskeleton)
      const flankW = bottomRadius * 0.14;
      const flankH = length * 0.88;
      const flankD = (bottomRadius + topRadius) * 0.82;
      [-bottomRadius * 0.68, bottomRadius * 0.68].forEach(xSign => {
        const flankGeo = new THREE.BoxGeometry(flankW, flankH, flankD);
        const flank = new THREE.Mesh(flankGeo, materials.armor);
        flank.position.set(xSign, midY, -bottomRadius * 0.04);
        flank.castShadow = true;
        flank.receiveShadow = true;
        parentJoint.add(flank);

        // CNC lightening cutouts on side plates
        const cutoutGeo = new THREE.BoxGeometry(flankW * 1.2, length * 0.38, flankD * 0.42);
        const cutout = new THREE.Mesh(cutoutGeo, materials.darkFrame);
        cutout.position.set(xSign, midY, -bottomRadius * 0.04);
        parentJoint.add(cutout);
      });

      // 3. Dorsal Protective Carapace / Aerodynamic Shield
      const carapaceW = (bottomRadius + topRadius) * 1.15;
      const carapaceGeo = new THREE.BoxGeometry(carapaceW, length * 0.82, bottomRadius * 0.22);
      const carapace = new THREE.Mesh(carapaceGeo, materials.armor);
      carapace.position.set(0, midY, -bottomRadius * 0.46);
      carapace.castShadow = true;
      parentJoint.add(carapace);

      // 4. Volar (Palmar) Tactile Grip Pad with Segmented Friction Tread
      const padW = (bottomRadius + topRadius) * 0.95;
      const padL = length * 0.75;
      const padD = bottomRadius * 0.42;
      const padGeo = new THREE.BoxGeometry(padW, padL, padD);
      const gripPad = new THREE.Mesh(padGeo, materials.sensorPad);
      gripPad.position.set(0, midY, bottomRadius * 0.44);
      gripPad.castShadow = true;
      parentJoint.add(gripPad);

      // Segmented micro-ribs along the volar pad
      const numRibs = phalanxIndex === 0 ? 3 : 2;
      for (let r = 0; r < numRibs; r++) {
        const ribY = midY - padL * 0.32 + (padL * 0.64 / (numRibs - 1 || 1)) * r;
        const ribGeo = new THREE.BoxGeometry(padW * 0.88, 0.035, 0.05);
        const ribMesh = new THREE.Mesh(ribGeo, materials.armor);
        ribMesh.position.set(0, ribY, bottomRadius * 0.66);
        parentJoint.add(ribMesh);
      }

      // 5. Dual Tendon Micro-Cables (Dyneema routing along the palmar flanks)
      [-bottomRadius * 0.48, bottomRadius * 0.48].forEach(xPos => {
        const tendonGeo = new THREE.CylinderGeometry(0.016, 0.016, length * 0.95, 6);
        const tendon = new THREE.Mesh(tendonGeo, materials.tendonWire);
        tendon.position.set(xPos, midY, bottomRadius * 0.36);
        parentJoint.add(tendon);
      });
    }

    /***********************************************************************
     * 3. CONTROL DE CÁMARA 3D, VISTAS SNAP Y ZOOM
     ***********************************************************************/
    function updateCameraOrbitPosition() {
      const sinPhi = Math.sin(camOrbit.phi);
      camera.position.x = camOrbit.radius * sinPhi * Math.sin(camOrbit.theta);
      camera.position.y = camOrbit.targetY + camOrbit.radius * Math.cos(camOrbit.phi);
      camera.position.z = camOrbit.radius * sinPhi * Math.cos(camOrbit.theta);
      camera.lookAt(0, camOrbit.targetY, 0);
    }

    function setupViewportInteractions(canvas) {
      canvas.addEventListener('mousedown', (e) => {
        camOrbit.isDragging = true;
        camOrbit.prevX = e.clientX;
        camOrbit.prevY = e.clientY;
      });

      window.addEventListener('mousemove', (e) => {
        if (!camOrbit.isDragging) return;
        const dx = e.clientX - camOrbit.prevX;
        const dy = e.clientY - camOrbit.prevY;
        camOrbit.prevX = e.clientX;
        camOrbit.prevY = e.clientY;

        camOrbit.theta -= dx * 0.008;
        camOrbit.phi = Math.max(0.08, Math.min(3.05, camOrbit.phi - dy * 0.008));
        updateCameraOrbitPosition();

        document.querySelectorAll('.viewport-top-left .view-control-btn').forEach(b => b.classList.remove('active'));
      });

      window.addEventListener('mouseup', () => {
        camOrbit.isDragging = false;
      });

      canvas.addEventListener('wheel', (e) => {
        e.preventDefault();
        camOrbit.radius = Math.max(5.0, Math.min(20.0, camOrbit.radius + e.deltaY * 0.006));
        updateCameraOrbitPosition();
      }, { passive: false });

      canvas.addEventListener('touchstart', (e) => {
        if (e.touches.length === 1) {
          camOrbit.isDragging = true;
          camOrbit.prevX = e.touches[0].clientX;
          camOrbit.prevY = e.touches[0].clientY;
        }
      }, { passive: true });

      canvas.addEventListener('touchmove', (e) => {
        if (!camOrbit.isDragging || e.touches.length !== 1) return;
        const dx = e.touches[0].clientX - camOrbit.prevX;
        const dy = e.touches[0].clientY - camOrbit.prevY;
        camOrbit.prevX = e.touches[0].clientX;
        camOrbit.prevY = e.touches[0].clientY;

        camOrbit.theta -= dx * 0.008;
        camOrbit.phi = Math.max(0.08, Math.min(3.05, camOrbit.phi - dy * 0.008));
        updateCameraOrbitPosition();

        document.querySelectorAll('.viewport-top-left .view-control-btn').forEach(b => b.classList.remove('active'));
      }, { passive: true });

      canvas.addEventListener('touchend', () => {
        camOrbit.isDragging = false;
      });
    }

    function setCameraSnapView(view, btn) {
      document.querySelectorAll('.viewport-top-left .view-control-btn').forEach(b => b.classList.remove('active'));
      if (btn) btn.classList.add('active');

      if (view === 'iso') {
        camOrbit.theta = 0.35;
        camOrbit.phi = 1.35;
        camOrbit.radius = 10.5;
      } else if (view === 'top') {
        camOrbit.theta = 0;
        camOrbit.phi = 0.08;
        camOrbit.radius = 10.5;
      } else if (view === 'side') {
        camOrbit.theta = Math.PI / 2;
        camOrbit.phi = Math.PI / 2;
        camOrbit.radius = 10.5;
      } else if (view === 'front') {
        camOrbit.theta = 0;
        camOrbit.phi = Math.PI / 2;
        camOrbit.radius = 10.5;
      }
      updateCameraOrbitPosition();
    }

    function zoomIn() {
      camOrbit.radius = Math.max(5.0, camOrbit.radius * 0.88);
      updateCameraOrbitPosition();
    }

    function zoomOut() {
      camOrbit.radius = Math.min(20.0, camOrbit.radius * 1.14);
      updateCameraOrbitPosition();
    }

    function zoomReset() {
      camOrbit.theta = 0.35;
      camOrbit.phi = 1.35;
      camOrbit.radius = 10.5;
      updateCameraOrbitPosition();
      document.querySelectorAll('.viewport-top-left .view-control-btn').forEach(b => b.classList.remove('active'));
      const isoBtn = document.querySelector('.viewport-top-left .view-control-btn');
      if (isoBtn) isoBtn.classList.add('active');
    }

    function resetCameraView() {
      zoomReset();
    }
