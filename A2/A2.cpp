#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

#define PI 3.14159265f
#define MAX_FOV 160.0f
#define MIN_FOV 5.0f
#define FOV_FACTOR 1.0f

#define NEAR_FAR_FACTOR 0.1f


float vertices[24] = {
  -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
};

int edges[24] = {
  0, 1,
  1, 3,
  3, 2,
  2, 0,
  0, 4,
  1, 5,
  2, 6,
  3, 7,
  4, 5,
  5, 7,
  7, 6,
  6, 4
};


//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.reserve(kMaxVertices);
	colours.reserve(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f)),
  model(1.0f),
  view(1.0f),
  proj(1.0f),
  modelTranslation(0, 0, 0),
  rotation(0),
  leftDown(false),
  rightDown(false),
  middleDown(false),
  mode(ROTATE_MODEL),
  mouseLastX(0),
  mouseLastY(0),
  clippingPlanes{
    vec4(0, 0, 1.0f, 1.0f), vec4(0, 0, 1.0f, 0),
    vec4(0, 0, 10.0f, 1.0f), vec4(0, 0, -1.0f, 0),
    vec4(-1.0f, 0, 0, 1.0f), vec4(1.0f, 0, 0, 0),
    vec4(1.0f, 0, 0, 1.0f), vec4(-1.0f, 0, 0, 0),
    vec4(0, -1.0f, 0, 1.0f), vec4(0, 1.0f, 0, 0),
    vec4(0, 1.0f, 0, 1.0f), vec4(0, -1.0f, 0, 0)
  }
{
  proj[3][3] = 0;
  proj[2][3] = 1.0f;
  setFOV(30.0f);
  setNearAndFar(1.0f, 10.0f);
  model = translate(model, vec3(0, 0, 4.0f));
  viewPortTop = 768 * 0.05f;
  viewPortBottom = 768 * 0.95f;
  viewPortLeft = 768 * 0.05f;
  viewPortRight = 768 * 0.95f;
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);


	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}


void A2::drawGnomon(mat4 transformation) {
  vec4 lineStart(0, 0, 0, 1.0f);
  lineStart = transformation * lineStart;

  vec4 lineX(1.0f, 0, 0, 1.0f);
  lineX = transformation * lineX;
  vec4 lineY(0, 1.0f, 0, 1.0f);
  lineY = transformation * lineY;
  vec4 lineZ(0, 0, 1.0f, 1.0f);
  lineZ = transformation * lineZ;

  setLineColour(vec3(1.0f, 0, 0));
  drawLine(vec2(lineStart.x / lineStart.w, lineStart.y / lineStart.w), vec2(lineX.x / lineX.w, lineX.y / lineX.w));
  setLineColour(vec3(0, 1.0f, 0));
  drawLine(vec2(lineStart.x / lineStart.w, lineStart.y / lineStart.w), vec2(lineY.x / lineY.w, lineY.y / lineY.w));
  setLineColour(vec3(0, 0, 1.0f));
  drawLine(vec2(lineStart.x / lineStart.w, lineStart.y / lineStart.w), vec2(lineZ.x / lineZ.w, lineZ.y / lineZ.w));
}


bool A2::clip(vec4& A, vec4& B, int c) {
  float wecA = dot(A - clippingPlanes[c], clippingPlanes[c + 1]);
  float wecB = dot(B - clippingPlanes[c], clippingPlanes[c + 1]);

  if (wecA < 0 && wecB < 0) {
    return false;
  }
  if (!(wecA >= 0 && wecB >= 0)) {
    float t = wecA / (wecA - wecB);
    if (wecA < 0) {
      A = A + t*(B - A);
    } else {
      B = A + t*(B - A);
    }
  }
  return true;
}

void A2::setFOV(float fov) {
  if (FOV <= MIN_FOV && fov <= MIN_FOV) return;
  if (fov <= MIN_FOV) fov = MIN_FOV;
  if (FOV >= MAX_FOV && fov >= MAX_FOV) return;
  if (fov >= MAX_FOV) fov = MAX_FOV;
  FOV = fov;
  float theta = fov * PI / 180.0f;
  float cot = 1 / tan(theta / 2.0f);
  proj[0][0] = cot; // aspect is 1:1
  proj[1][1] = cot;
}

void A2::setNear(float n) {
  if (n >= clippingPlanes[2].z) {
    n = clippingPlanes[2].z;
  }
  setNearAndFar(n, clippingPlanes[2].z);
}

void A2::setFar(float f) {
  if (f <= clippingPlanes[0].z) {
    f = clippingPlanes[0].z;
  }
  setNearAndFar(clippingPlanes[0].z, f);
}

void A2::setNearAndFar(float n, float f) {
  float farMinusNear = f - n;
  proj[2][2] = (f + n) / farMinusNear;
  proj[3][2] = (-2.0f * f * n) / farMinusNear;
  clippingPlanes[0].z = n;
  clippingPlanes[2].z = f;
}
//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

  drawGnomon(proj * view);
  drawGnomon(proj * view * model);

	setLineColour(vec3(1.0f, 1.0f, 1.0f));

  float xScale = (viewPortRight - viewPortLeft) / m_windowWidth;
  float yScale = (viewPortBottom - viewPortTop) / m_windowHeight;
  float left = viewPortLeft * 2.0f / m_windowWidth - 1.0f;
  float bottom = (m_windowHeight - viewPortBottom) * 2.0f / m_windowHeight - 1.0f;
  float right = left + xScale * 2.0f;
  float top = bottom + yScale * 2.0f;
  float middleX = left + xScale;
  float middleY = bottom + yScale;
  mat4 viewPort(1.0f);
  viewPort = translate(viewPort, vec3(middleX, middleY, 0));



  drawLine(vec2(left, top), vec2(right, top));
  drawLine(vec2(right, top), vec2(right, bottom));
  drawLine(vec2(right, bottom), vec2(left, bottom));
  drawLine(vec2(left, bottom), vec2(left, top));

  for (int i = 0; i < 24; i+=2) {
    vec4 A(vertices[edges[i] * 3], vertices[edges[i] * 3 + 1], vertices[edges[i] * 3 + 2], 1.0f);
    vec4 B(vertices[edges[i + 1] * 3], vertices[edges[i + 1] * 3 + 1], vertices[edges[i + 1] * 3 + 2], 1.0f);

    A = view * model * A;
    B = view * model * B;


    bool draw = clip(A, B, 0); //near-field clip



    A = proj * A;
    B = proj * B;
    A.x /= A.w;
    A.y /= A.w;
    B.x /= B.w;
    B.y /= B.w;


    for (int c = 2; c < 12; c += 2) {
      if (!clip(A, B, c)) {
        draw = false;
        break;
      }
    }

    if (draw) {
      A = viewPort * A;
      B = viewPort * B;
      drawLine(
        vec2(A.x, A.y),
        vec2(B.x, B.y));
    }
  }
  // lineStart = proj * lineStart;
  // lineEnd = proj * lineStart;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text( "Near field: %.1f", clippingPlanes[0].z );
		ImGui::Text( "Far field: %.1f", clippingPlanes[2].z );

		ImGui::Text( "mousey: %.1f", mouseLastY );


      float xScale = (viewPortRight - viewPortLeft) * 2.0f / m_windowWidth;
      float yScale = (viewPortTop - viewPortBottom) * 2.0f / m_windowWidth;
      float left = viewPortLeft * 2.0f / m_windowWidth - 1.0f;
      float top = viewPortTop * 2.0f / m_windowHeight - 1.0f;

      		ImGui::Text( "xScale: %.1f", xScale );
          		ImGui::Text( "left: %.1f", left );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);


  if (!ImGui::IsMouseHoveringAnyWindow()) {
    float xOffset = xPos - mouseLastX;
    switch(mode) {
      case(ROTATE_VIEW):
        if (leftDown) {
          view = glm::rotate(view, 0.1f * xOffset, vec3(1, 0, 0));
        }
        if (middleDown) {
          view = glm::rotate(view, 0.1f * xOffset, vec3(0, 1, 0));
        }
        if (rightDown) {
          view = glm::rotate(view, 0.1f * xOffset, vec3(0, 0, 1));
        }
        break;
      case(TRANSLATE_VIEW):
        if (leftDown) {
          view = glm::translate(view, vec3(0.1f * xOffset, 0, 0));
        }
        if (middleDown) {
          view = glm::translate(view, vec3(0, 0.1f * xOffset, 0));
        }
        if (rightDown) {
          view = glm::translate(view, vec3(0, 0, 0.1f * xOffset));
        }
        break;

      case(ROTATE_MODEL):
        if (leftDown) {
          model = glm::rotate(model, 0.1f * xOffset, vec3(1, 0, 0));
        }
        if (middleDown) {
          model = glm::rotate(model, 0.1f * xOffset, vec3(0, 1, 0));
        }
        if (rightDown) {
          model = glm::rotate(model, 0.1f * xOffset, vec3(0, 0, 1));
        }
        break;
      case(TRANSLATE_MODEL):
        if (leftDown) {
          model = glm::translate(model, vec3(0.1f * xOffset, 0, 0));
        }
        if (middleDown) {
          model = glm::translate(model, vec3(0, 0.1f * xOffset, 0));
        }
        if (rightDown) {
          model = glm::translate(model, vec3(0, 0, 0.1f * xOffset));
        }
        break;
      case(SCALE_MODEL):
        if (leftDown) {
          model = glm::scale(model, vec3(pow(1.1f, xOffset), 1.0f, 1.0f));
        }
        if (middleDown) {
          model = glm::scale(model, vec3(1.0f, pow(1.1f, xOffset), 1.0f));
        }
        if (rightDown) {
          model = glm::scale(model, vec3(1.0f, 1.0f, pow(1.1f, xOffset)));
        }
        break;
      case(PERSPECTIVE):
        if (leftDown) {
          setFOV(FOV + xOffset * FOV_FACTOR);
        }
        if (middleDown) {
          setNear(clippingPlanes[0].z + xOffset * NEAR_FAR_FACTOR);
        }
        if (rightDown) {
          setFar(clippingPlanes[2].z + xOffset * NEAR_FAR_FACTOR);
        }
        break;
      case(VIEWPORT):
        if (leftDown) {
          viewPortRight = xPos;
          viewPortBottom = yPos;
        }
        break;
    }

    mouseLastX = xPos;
    mouseLastY = yPos;
  }
	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int action,
		int mods
) {
	bool eventHandled(false);
  if (!ImGui::IsMouseHoveringAnyWindow()) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (action == GLFW_PRESS) {
        if (mode == VIEWPORT) {
          viewPortLeft = mouseLastX;
          viewPortTop = mouseLastY;
        }
        leftDown = true;
      }
      if (action == GLFW_RELEASE) {
        leftDown = false;
      }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      if (action == GLFW_PRESS) {
        rightDown = true;
      }
      if (action == GLFW_RELEASE) {
        rightDown = false;
      }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
      if (action == GLFW_PRESS) {
        middleDown = true;
      }
      if (action == GLFW_RELEASE) {
        middleDown = false;
      }
    }
  }


	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
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
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

  if( action == GLFW_PRESS ) {
    switch(key) {
     case(GLFW_KEY_O):
      mode = ROTATE_VIEW;
      eventHandled = true;
      break;
     case(GLFW_KEY_N):
      mode = TRANSLATE_VIEW;
      eventHandled = true;
      break;
     case(GLFW_KEY_R):
      mode = ROTATE_MODEL;
      eventHandled = true;
      break;
     case(GLFW_KEY_T):
      mode = TRANSLATE_MODEL;
      eventHandled = true;
      break;
     case(GLFW_KEY_S):
      mode = SCALE_MODEL;
      eventHandled = true;
      break;
     case(GLFW_KEY_P):
      mode = PERSPECTIVE;
      eventHandled = true;
      break;
     case(GLFW_KEY_V):
      mode = VIEWPORT;
      eventHandled = true;
      break;
     }
  }

	// Fill in with event handling code...

	return eventHandled;
}
