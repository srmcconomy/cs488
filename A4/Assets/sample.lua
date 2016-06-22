purpleorb = gr.material({0.4, 0, 0.8}, {0.3, 0.3, 0.3}, 25)
greenorb = gr.material({0, 0.8, 0}, {0.3, 0.3, 0.3}, 25)
redorb = gr.material({0.8, 0, 0}, {0.3, 0.3, 0.3}, 25)
gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25)


scene = gr.node('scene')
scene:rotate('X', 23)

orb1 = gr.sphere('orb1')
scene:add_child(orb1)
orb1:scale(2, 2, 2)
orb1:translate(0, 10, 0)
orb1:set_material(purpleorb)

orb2 = gr.sphere('orb2')
scene:add_child(orb2)
orb2:translate(3, 6, 1)
orb2:set_material(greenorb)

orb3 = gr.sphere('orb3')
scene:add_child(orb3)
orb3:scale(5, 5, 5)
orb3:translate(-7, 2, -2)
orb3:set_material(redorb)

cow = gr.mesh('cow', 'cow.obj')
scene:add_child(cow)
cow:rotate('X', -40)
cow:rotate('Y', -20)
cow:set_material(gold)

redlight = gr.light({100, 100, 100}, {0.8, 0, 0}, {1, 0, 0})
whitelight = gr.light({-100, 100, 100}, {0.8, 0.8, 0.8}, {1, 0, 0})
bluelight = gr.light({100, 100, -100}, {0, 0, 0.8}, {1, 0, 0})

gr.render(scene,
	  'sample.png', 256, 256,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {redlight, whitelight, bluelight})
