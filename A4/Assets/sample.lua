-- A more macho version of simple_cows.py, in which cows aren't
-- spheres, they're cow-shaped polyhedral models.


-- We'll need an extra function that knows how to read Wavefront .OBJ
-- files.

stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)

mouse = gr.material({0.1,0.1, 0.1}, {0.7, 0.7, 0.7}, 100)

-- ##############################################
-- the arch
-- ##############################################

inst = gr.node('inst')

arc = gr.node('arc')
inst:add_child(arc)
arc:translate(0, 0, -10)

p1 = gr.cube('p1', {0, 0, 0}, 1)
arc:add_child(p1)
p1:set_material(stone)
p1:scale(0.8, 4, 0.8)
p1:translate(-2.4, 0, -0.4)

p2 = gr.cube('p2', {0, 0, 0}, 1)
arc:add_child(p2)
p2:set_material(stone)
p2:scale(0.8, 4, 0.8)
p2:translate(1.6, 0, -0.4)

s = gr.sphere('s', {0, 0, 0}, 1)
arc:add_child(s)
s:set_material(stone)
s:scale(4, 0.6, 0.6)
s:translate(0, 4, 0)

cow_poly = gr.mesh('cow', 'cow.obj')
factor = 2.0/(2.76+3.637)

cow_poly:set_material(hide)

cow_poly:translate(0.0, 3.637, 0.0)
cow_poly:scale(factor, factor, factor)
cow_poly:translate(0.0, -1.0, 0.0)

-- ##############################################
-- the scene
-- ##############################################

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh('plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(grass)
plane:scale(30, 30, 30)
plane:rotate('X', 180)

mickey = gr.mesh( 'mickey', 'mickey.obj' )
scene:add_child(mickey)
mickey:set_material(mouse)
mickey:rotate('X', -90);
mickey:rotate('Y', -90);
mickey:scale(5, 5, 5)

icosa = gr.mesh( 'icosa', 'dodeca.obj' )
scene:add_child(icosa)
icosa:set_material(stone)
icosa:scale(1.5, 1.5, 1.5)
icosa:rotate('Y', 20)
icosa:translate(0, 10, 0);

cow_instance = gr.node('cow')
scene:add_child(cow_instance)
cow_instance:add_child(cow_poly)
cow_instance:scale(2, 2, 2)
cow_instance:rotate('Y', 20)
cow_instance:translate(1, 1.3, 14)

cow_instance2 = gr.node('cow2')
scene:add_child(cow_instance2)
cow_instance2:add_child(cow_poly)
cow_instance2:rotate('Y', 150)
cow_instance2:translate(6, 1.3, -4)

cow_instance3 = gr.node('cow3')
scene:add_child(cow_instance3)
cow_instance3:add_child(cow_poly)
cow_instance3:rotate('Y', 150)
cow_instance3:translate(6, 1.3, -4)

cylinder = gr.mesh('cylinder', 'cylinder.obj')
scene:add_child(cylinder)
cylinder:translate(-10, 0, 0)
cylinder:scale(10, 10, 10)

gr.render(scene,
	  'sample.png', 256, 256,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0}),gr.light({500, 0, 100}, {0.8, 0.2, 0.2}, {1, 0, 0})})
