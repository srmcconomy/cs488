purpleorb = gr.material({0.5, 0, 1.0}, {0.3, 0.3, 0.3}, 100)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)


scene = gr.node('scene')
scene:rotate('X', 23)

orb1 = gr.sphere('orb1')
scene:add_child(orb1)
orb1:scale(10, 10, 10)
orb1:translate(0, 10, 0)
orb1:set_material(purpleorb)

redlight = gr.light({100, 100, 100}, {0.8, 0, 0}, {1, 0, 0})
whitelight = gr.light({-100, 100, 100}, {0.8, 0.8, 0.8}, {1, 0, 0})
bluelight = gr.light({100, 100, -100}, {0, 0, 0.8}, {1, 0, 0})

gr.render(scene,
	  'sample.png', 256, 256,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {redlight, whitelight, bluelight})
