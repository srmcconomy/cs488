#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;
const float TRANSLATE_FACTOR = 0.001f;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
		m_pickingPositionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
    translateTrans(1.0f),
    rotationTrans(1.0f),
    mouseLeftDown(false),
    mouseMiddleDown(false),
    mouseRightDown(false),
		mode(POSITION),
		backCulling(false),
		frontCulling(false),
		drawCircle(false),
		zbuffering(true)
{
  matrixStack.push(mat4(1.0f));
}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
  trackballOrigin = vec2(m_framebufferWidth / 2.0f, m_framebufferHeight / 2.0f);
  trackballRadius = std::min(m_framebufferWidth / 4.0f, m_framebufferHeight / 4.0f);
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();

	m_picking_shader.generateProgramObject();
	m_picking_shader.attachVertexShader( getAssetFilePath("PickingVertexShader.vs").c_str() );
	m_picking_shader.attachFragmentShader( getAssetFilePath("PickingFragmentShader.fs").c_str() );
	m_picking_shader.link();

}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}

	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	{
		glBindVertexArray(m_vao_meshData);
		m_pickingPositionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_pickingPositionAttribLocation);

	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	glBindVertexArray(m_vao_meshData);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_pickingPositionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.05f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();

	m_picking_shader.enable();
	{
		GLint location = m_picking_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;
	}
	m_picking_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
	float opacity(0.5f);


	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Application")) {
				if (ImGui::MenuItem("Reset Position")) {
					resetPosition();
				}
				if (ImGui::MenuItem("Reset Orientation")) {
					resetOrientation();
				}
				if (ImGui::MenuItem("Reset Joints")) {
					resetJoints();
				}
				if (ImGui::MenuItem("Reset All")) {
					resetAll();
				}
				if (ImGui::MenuItem("Quit")) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo")) {
					resetPosition();
				}
				if (ImGui::MenuItem("Redo")) {
					resetOrientation();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Options")) {
				ImGui::Checkbox("Circle", &drawCircle);
				ImGui::Checkbox("Z-buffer", &zbuffering);
				ImGui::Checkbox("Backface Culling", &backCulling);
				ImGui::Checkbox("Frontface Culling", &frontCulling);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::RadioButton( "Position/Orientation", (int*)&mode, POSITION );
		ImGui::RadioButton( "Joints", (int*)&mode, JOINTS );

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		bool picking
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		if (picking) {
			location = shader.getUniformLocation("PickingColor");
			vec4 colour = vec4((float)node.m_nodeId / 255.0f, 0, 0, 1.0f);
			glUniform4fv(location, 1, value_ptr(colour));
		} else {

			// -- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;



			// -- Set Material values:
			location = shader.getUniformLocation("material.kd");
			vec3 kd = node.material.kd;
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.ks");
			vec3 ks = node.material.ks;
			if (node.isSelected) {
				ks = {0, 0, 1.0f};
			}
			glUniform3fv(location, 1, value_ptr(ks));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.shininess");
			glUniform1f(location, node.material.shininess);
			CHECK_GL_ERRORS;
		}

	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	if (zbuffering) {
		glEnable( GL_DEPTH_TEST );
	}
	if (frontCulling && backCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
	} else if (frontCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} else if (backCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	renderSceneGraph(*m_rootNode, false);

	if (zbuffering) {
		glDisable( GL_DEPTH_TEST );
	}

	if (frontCulling || backCulling) {
		glDisable( GL_CULL_FACE );
	}
	if (drawCircle) {
		renderArcCircle();
	}
}

void A3::renderNode(const SceneNode * node, const ShaderProgram& shader, bool picking) {
  if (node->m_nodeType == NodeType::GeometryNode) {
    const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

		updateShaderUniforms(shader, *geometryNode, m_view * translateTrans * rotationTrans * matrixStack.top(), picking);

		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];
		//-- Now render the mesh:
		shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		shader.disable();
  }
  matrixStack.push(matrixStack.top() * node->trans);
  for (const SceneNode* child : node->children) {
    renderNode(child, shader, picking);
  }
  matrixStack.pop();
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root, bool picking) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);
  renderNode(&root, picking ? m_picking_shader : m_shader, picking);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}


void A3::resetPosition() {
	translateTrans = mat4(1.0f);
}

void A3::resetOrientation() {
	rotationTrans = mat4(1.0f);
}

void A3::resetJoints() {

}

void A3::resetAll() {
	resetPosition();
	resetOrientation();
	resetJoints();
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}


float clampf(float a, float min, float max) {
  if (a > max) return max;
  if (a < min) return min;
  return a;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	  double xOffset = xPos - mouseLastX;
	  double yOffset = yPos - mouseLastY;
	if (mode == POSITION) {

	  vec3 trackball = vec3(xPos - trackballOrigin.x, -yPos + trackballOrigin.y, 0);
	  trackball /= trackballRadius;
	  float sqlength = trackball.x * trackball.x + trackball.y * trackball.y;
	  if (sqlength > 1) {
	    trackball /= sqrt(sqlength);
	  } else {
	    trackball.z = sqrt(1 - sqlength);
	  }

	  if (mouseLeftDown) {
	  	// Fill in with event handling code...
	    translateTrans = translate(translateTrans, vec3(xOffset * TRANSLATE_FACTOR, -yOffset * TRANSLATE_FACTOR, 0));
	    eventHandled = true;
	  }

	  if (mouseMiddleDown) {
	    translateTrans = translate(translateTrans, vec3(0, 0, yOffset * TRANSLATE_FACTOR));
	    eventHandled = true;
	  }

	  if (mouseRightDown) {
	    float angle = acos(clampf(dot(trackball, lastTrackball), 0, 1.0f));
	    if (angle > 0)
	      rotationTrans = rotate(mat4(1.0f), angle, -1.0f * cross(trackball, lastTrackball)) * rotationTrans;
	    eventHandled = true;
	  }
	  lastTrackball = trackball;
	} else if (mode == JOINTS) {
		if (mouseRightDown) {
			for (map<unsigned int, SceneNode*>::iterator it = selectedJoints.begin(); it != selectedJoints.end(); it++) {
				//it->second->rotate('x', yOffset * 0.001f);
			}
		}
	}
  mouseLastX = xPos;
  mouseLastY = yPos;

	return eventHandled;
}


//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);
	double xPos;
	double yPos;
	glfwGetCursorPos(m_window, &xPos, &yPos);

	switch(mode) {
		case POSITION:
		  if (actions == GLFW_PRESS) {
		    if (button == GLFW_MOUSE_BUTTON_LEFT) {
		      mouseLeftDown = true;
		    }
		    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		      mouseMiddleDown = true;
		    }
		    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		      mouseRightDown = true;
		    }
		  } else if (actions == GLFW_RELEASE) {
		    if (button == GLFW_MOUSE_BUTTON_LEFT) {
		      mouseLeftDown = false;
		    }
		    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		      mouseMiddleDown = false;
		    }
		    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		      mouseRightDown = false;
		    }
		  }
			break;
		case JOINTS:
			if (actions == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				if (zbuffering) {
					glEnable( GL_DEPTH_TEST );
				}
				renderSceneGraph(*m_rootNode, true);
				if (zbuffering) {
					glDisable( GL_DEPTH_TEST );
				}
				glFlush();
				glFinish();
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				unsigned char data[4];
				glReadPixels(xPos, m_framebufferHeight - yPos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
				if (data[1] == 0) {
					SceneNode* node = SceneNode::getNode((unsigned int)data[0]);
					if (node->parent->m_nodeType == NodeType::JointNode) {
						node->isSelected = !node->isSelected;
						if (node->isSelected) {
							selectedJoints[node->parent->m_nodeId] = (JointNode*)node->parent;
						} else {
							selectedJoints.erase(node->parent->m_nodeId);
						}
					}
				}
			}
			if (actions == GLFW_PRESS) {
		    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		      mouseMiddleDown = true;
		    }
		    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		      mouseRightDown = true;
		    }
		  } else if (actions == GLFW_RELEASE) {
		    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		      mouseMiddleDown = false;
		    }
		    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		      mouseRightDown = false;
		    }
		  }
			break;
	}


	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		if( key == GLFW_KEY_B ) {
			backCulling = !backCulling;
		}
		if( key == GLFW_KEY_F ) {
			frontCulling = !frontCulling;
		}
		if( key == GLFW_KEY_C ) {
			drawCircle = !drawCircle;
		}
		if( key == GLFW_KEY_Z ) {
			zbuffering = !zbuffering;
		}
		if( key == GLFW_KEY_I) {
			resetPosition();
		}
		if( key == GLFW_KEY_O) {
			resetOrientation();
		}
		if( key == GLFW_KEY_N) {
			resetJoints();
		}
		if( key == GLFW_KEY_A) {
			resetAll();
		}
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if (key == GLFW_KEY_P) {
			mode = POSITION;
		}
		if (key == GLFW_KEY_J) {
			mode = JOINTS;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
