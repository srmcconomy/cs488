-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
rootnode:rotate('y', -20.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, 0.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('cube', 'torso')
rootnode:add_child(torso)
torso:set_material(white)
torso:scale(0.5,0.5,0.5);

neck = gr.mesh('sphere', 'neck')
torso:add_child(neck)
neck:scale(1.0/0.5, 2, 1.0/0.5)
neck:scale(0.15, 0.3, 0.15)
neck:translate(0.0, 0.6, 0.0)
neck:set_material(blue)

neckNode = gr.node('neckNode')
neck:add_child(neckNode)
neckNode:scale(1/0.15, 1/0.3, 1/0.15)

neckJoint = gr.joint('neckJoint', {-10, 0, 90}, {-110, 0, 110});
neckNode:add_child(neckJoint)

head = gr.mesh('cube', 'head')
neckJoint:add_child(head)
head:scale(0.4, 0.4, 0.4)
head:translate(0.0, 0.3, 0.0)
head:set_material(red)

ears = gr.mesh('sphere', 'ears')
head:add_child(ears)
ears:scale(1.2, 0.08, 0.08)
ears:set_material(red)
ears:set_material(blue)

leftEye = gr.mesh('cube', 'leftEye')
head:add_child(leftEye)
leftEye:scale(0.2, 0.1, 0.1)
leftEye:translate(-0.2, 0.2, 0.5)
leftEye:set_material(blue)

rightEye = gr.mesh('cube', 'rightEye')
head:add_child(rightEye)
rightEye:scale(0.2, 0.1, 0.1)
rightEye:translate(0.2, 0.2, 0.5)
rightEye:set_material(blue)

leftShoulder = gr.mesh('sphere', 'leftShoulder')
torso:add_child(leftShoulder)
leftShoulder:scale(1/0.5,2,1/0.5);
leftShoulder:scale(0.2, 0.2, 0.2)
leftShoulder:translate(-0.4, 0.35, 0.0)
leftShoulder:set_material(blue)

leftShoulderNode = gr.node('leftShoulderNode')
leftShoulder:add_child(leftShoulderNode)
leftShoulderNode:scale(5, 5, 5);
leftShoulderNode:rotate('y', -90);

leftShoulderJoint = gr.joint('leftShoulderJoint', {-90, 50, 90}, {-20, 0, 90})
leftShoulderNode:add_child(leftShoulderJoint)

leftArm = gr.mesh('cube', 'leftArm')
leftShoulderJoint:add_child(leftArm)
leftArm:translate(0.0, 0.0, 0.5)
leftArm:scale(0.1, 0.1, 0.4)
leftArm:set_material(red)

leftElbowNode = gr.node('leftElbowNode')
leftArm:add_child(leftElbowNode);
leftElbowNode:scale(10, 10, 2.25);
leftElbowNode:translate(0, 0, 0.4);

leftElbowJoint = gr.joint('leftElbowJoint', {-170, -20, 0}, {0, 0, 0})
leftElbowNode:add_child(leftElbowJoint)

leftForearm = gr.mesh('cube', 'leftForearm')
leftElbowJoint:add_child(leftForearm)
leftForearm:translate(0.0, 0.0, 0.5)
leftForearm:scale(0.1, 0.1, 0.4)
leftForearm:set_material(red)

leftWristNode = gr.node('leftWristNode')
leftForearm:add_child(leftWristNode);
leftWristNode:scale(10, 10, 2.25);
leftWristNode:translate(0, 0, 0.4);

leftWristJoint = gr.joint('leftWristJoint', {-90, 20, 90}, {0, 0, 0})
leftWristNode:add_child(leftWristJoint)

leftHand = gr.mesh('cube', 'leftHand')
leftWristJoint:add_child(leftHand)
leftHand:translate(0.0, 0.0, 0.5)
leftHand:scale(0.1, 0.1, 0.2)
leftHand:set_material(red)

rightShoulder = gr.mesh('sphere', 'rightShoulder')
torso:add_child(rightShoulder)
rightShoulder:scale(1/0.5,2,1/0.5);
rightShoulder:scale(0.2, 0.2, 0.2)
rightShoulder:translate(0.4, 0.35, 0.0)
rightShoulder:set_material(blue)

rightShoulderNode = gr.node('rightShoulderNode')
rightShoulder:add_child(rightShoulderNode)
rightShoulderNode:scale(5, 5, 5);
rightShoulderNode:rotate('y', 90);

rightShoulderJoint = gr.joint('rightShoulderJoint', {-90, 50, 90}, {-20, 0, 90})
rightShoulderNode:add_child(rightShoulderJoint)

rightArm = gr.mesh('cube', 'rightArm')
rightShoulderJoint:add_child(rightArm)
rightArm:translate(0.0, 0.0, 0.5)
rightArm:scale(0.1, 0.1, 0.4)
rightArm:set_material(red)

rightElbowNode = gr.node('rightElbowNode')
rightArm:add_child(rightElbowNode);
rightElbowNode:scale(10, 10, 2.25);
rightElbowNode:translate(0, 0, 0.4);

rightElbowJoint = gr.joint('rightElbowJoint', {-170, -20, 0}, {0, 0, 0})
rightElbowNode:add_child(rightElbowJoint)

rightForearm = gr.mesh('cube', 'rightForearm')
rightElbowJoint:add_child(rightForearm)
rightForearm:translate(0.0, 0.0, 0.5)
rightForearm:scale(0.1, 0.1, 0.4)
rightForearm:set_material(red)

rightWristNode = gr.node('rightWristNode')
rightForearm:add_child(rightWristNode);
rightWristNode:scale(10, 10, 2.25);
rightWristNode:translate(0, 0, 0.4);

rightWristJoint = gr.joint('rightWristJoint', {-90, 20, 90}, {0, 0, 0})
rightWristNode:add_child(rightWristJoint)

rightHand = gr.mesh('cube', 'rightHand')
rightWristJoint:add_child(rightHand)
rightHand:translate(0.0, 0.0, 0.5)
rightHand:scale(0.1, 0.1, 0.2)
rightHand:set_material(red)

waistNode = gr.node('waistNode')
torso:add_child(waistNode)
waistNode:scale(1/0.5, 2, 1/.5)
waistNode:translate(0, -0.5, 0)

waistJoint = gr.joint('waistJoint', {0, 0, 0}, {-90, 20, 90})
waistNode:add_child(waistJoint);

lowerTorso = gr.mesh('cube', 'lowerTorso')
waistJoint:add_child(lowerTorso)
lowerTorso:scale(0.5, 0.5, 0.5)
lowerTorso:set_material(white);
lowerTorso:translate(0, -0.25, 0)

leftHip = gr.mesh('sphere', 'leftHip')
lowerTorso:add_child(leftHip)
leftHip:scale(1/0.5,2,1/0.5);
leftHip:scale(0.21, 0.21, 0.21)
leftHip:translate(-0.38, -0.5, 0.0)
leftHip:set_material(blue)

leftHipNode = gr.node('leftHipNode')
leftHip:add_child(leftHipNode)
leftHipNode:scale(1/0.21, 1/0.21, 1/0.21);

leftHipJoint = gr.joint('leftHipJoint', {-90, -30, 90}, {0, 0, 0})
leftHipNode:add_child(leftHipJoint)

leftThigh = gr.mesh('cube', 'leftThigh')
leftHipJoint:add_child(leftThigh)
leftThigh:translate(0, -0.5, 0)
leftThigh:scale(0.1, 0.7, 0.1)
leftThigh:set_material(red)

leftKneeNode = gr.node('leftKneeNode')
leftThigh:add_child(leftKneeNode)
leftKneeNode:scale(10, 1/0.7, 10)
leftKneeNode:translate(0, -0.5, 0)

leftKneeJoint = gr.joint('leftKneeJoint', {0, 10, 170}, {0, 0, 0})
leftKneeNode:add_child(leftKneeJoint)

leftCalf = gr.mesh('cube', 'leftCalf')
leftKneeJoint:add_child(leftCalf)
leftCalf:translate(0, -0.5, 0)
leftCalf:scale(0.1, 0.7, 0.1)
leftCalf:set_material(red)

leftAnkleNode = gr.node('leftAnkleNode')
leftCalf:add_child(leftAnkleNode)
leftAnkleNode:scale(10, 1/0.7, 10)
leftAnkleNode:translate(0, -0.5, 0)

leftAnkleJoint = gr.joint('leftAnkleJoint', {-120, -90, -20}, {0, 0, 0})
leftAnkleNode:add_child(leftAnkleJoint)

leftFoot = gr.mesh('cube', 'leftFoot')
leftAnkleJoint:add_child(leftFoot)
leftFoot:translate(0, -0.5, 0)
leftFoot:scale(0.1, 0.3, 0.1)
leftFoot:set_material(red)

rightHip = gr.mesh('sphere', 'rightHip')
lowerTorso:add_child(rightHip)
rightHip:scale(1/0.5,2,1/0.5);
rightHip:scale(0.21, 0.21, 0.21)
rightHip:translate(0.38, -0.5, 0.0)
rightHip:set_material(blue)

rightHipNode = gr.node('rightHipNode')
rightHip:add_child(rightHipNode)
rightHipNode:scale(1/0.21, 1/0.21, 1/0.21);

rightHipJoint = gr.joint('rightHipJoint', {-90, 30, 90}, {0, 0, 0})
rightHipNode:add_child(rightHipJoint)

rightThigh = gr.mesh('cube', 'rightThigh')
rightHipJoint:add_child(rightThigh)
rightThigh:translate(0, -0.5, 0)
rightThigh:scale(0.1, 0.7, 0.1)
rightThigh:set_material(red)

rightKneeNode = gr.node('rightKneeNode')
rightThigh:add_child(rightKneeNode)
rightKneeNode:scale(10, 1/0.7, 10)
rightKneeNode:translate(0, -0.5, 0)

rightKneeJoint = gr.joint('rightKneeJoint', {0, 40, 170}, {0, 0, 0})
rightKneeNode:add_child(rightKneeJoint)

rightCalf = gr.mesh('cube', 'rightCalf')
rightKneeJoint:add_child(rightCalf)
rightCalf:translate(0, -0.5, 0)
rightCalf:scale(0.1, 0.7, 0.1)
rightCalf:set_material(red)


rightAnkleNode = gr.node('rightAnkleNode')
rightCalf:add_child(rightAnkleNode)
rightAnkleNode:scale(10, 1/0.7, 10)
rightAnkleNode:translate(0, -0.5, 0)

rightAnkleJoint = gr.joint('rightAnkleJoint', {-120, -90, -20}, {0, 0, 0})
rightAnkleNode:add_child(rightAnkleJoint)

rightFoot = gr.mesh('cube', 'rightFoot')
rightAnkleJoint:add_child(rightFoot)
rightFoot:translate(0, -0.5, 0)
rightFoot:scale(0.1, 0.3, 0.1)
rightFoot:set_material(red)


return rootnode
