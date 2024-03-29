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
#define ROTATE_FACTOR 0.2f
#define SCALE_FACTOR 0.1f
#define TRANSLATE_FACTOR 0.2f


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
  modelScale(1.0f),
  view(1.0f),
  viewRotation(1.0f),
  proj(1.0f),
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
  viewPortY2 = 768 * 0.05f;
  viewPortY1 = 768 * 0.95f;
  viewPortX1 = 768 * 0.05f;
  viewPortX2 = 768 * 0.95f;
  view = translate(mat4(1.0f), vec3(0, 0, 5.0f));
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

void divideByW(vec4& v) {
  v.x /= v.w;
  v.y /= v.w;
}


void A2::drawGnomon(mat4 transformation, mat4 projection, float scaleX, float scaleY, float middleX, float middleY) {
  vec4 lineStart(0, 0, 0, 1.0f);
  lineStart = transformation * lineStart;

  vec4 lineStartX = lineStart;
  vec4 lineStartY = lineStart;
  vec4 lineStartZ = lineStart;

  vec4 lineX(1.0f, 0, 0, 1.0f);
  lineX = transformation * lineX;
  vec4 lineY(0, 1.0f, 0, 1.0f);
  lineY = transformation * lineY;
  vec4 lineZ(0, 0, 1.0f, 1.0f);
  lineZ = transformation * lineZ;

  bool drawX = clip(lineStartX, lineX, 0);
  bool drawY = clip(lineStartY, lineY, 0);
  bool drawZ = clip(lineStartZ, lineZ, 0);
  if (drawX) {
    lineStartX = projection * lineStartX;
    lineX = projection * lineX;
    divideByW(lineStartX);
    divideByW(lineX);
  }
  if (drawY) {
    lineStartY = projection * lineStartY;
    lineY = projection * lineY;
    divideByW(lineStartY);
    divideByW(lineY);
  }
  if (drawZ) {
    lineStartZ = projection * lineStartZ;
    lineZ = projection * lineZ;
    divideByW(lineStartZ);
    divideByW(lineZ);
  }

  for (int c = 2; c < 12; c+=2) {
    if (drawX) drawX &= clip(lineStartX, lineX, c);
    if (drawY) drawY &= clip(lineStartY, lineY, c);
    if (drawZ) drawZ &= clip(lineStartZ, lineZ, c);
  }

  if (drawX) {
    setLineColour(vec3(1.0f, 0, 0));
    drawLine(vec2(lineStartX.x * scaleX + middleX, lineStartX.y * scaleY + middleY), vec2(lineX.x * scaleX + middleX, lineX.y * scaleY + middleY));
  }
  if (drawY) {
    setLineColour(vec3(0, 1.0f, 0));
    drawLine(vec2(lineStartY.x * scaleX + middleX, lineStartY.y * scaleY + middleY), vec2(lineY.x * scaleX + middleX, lineY.y * scaleY + middleY));
  }
  if (drawZ) {
    setLineColour(vec3(0, 0, 1.0f));
    drawLine(vec2(lineStartZ.x * scaleX + middleX, lineStartZ.y * scaleY + middleY), vec2(lineZ.x * scaleX + middleX, lineZ.y * scaleY + middleY));
  }
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

void A2::reset() {
  model = mat4(1.0f);
  modelScale = mat4(1.0f);
  view = translate(view, vec3(0, 0, 5.0f));
  viewRotation = mat4(1.0f);
  leftDown = false;
  rightDown = false;
  middleDown = false;
  mode = ROTATE_MODEL;
  setNearAndFar(1.0f, 10.0f);
  setFOV(30.0f);
  viewPortY2 = 768 * 0.05f;
  viewPortY1 = 768 * 0.95f;
  viewPortX1 = 768 * 0.05f;
  viewPortX2 = 768 * 0.95f;
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

  float viewPortLeft = viewPortX1 < viewPortX2 ? viewPortX1 : viewPortX2;
  float viewPortRight = viewPortX1 < viewPortX2 ? viewPortX2 : viewPortX1;
  float viewPortBottom = viewPortY1 < viewPortY2 ? viewPortY1 : viewPortY2;
  float viewPortTop = viewPortY1 < viewPortY2 ? viewPortY2 : viewPortY1;
  float xScale = (viewPortRight - viewPortLeft) / m_framebufferWidth;
  float yScale = (viewPortBottom - viewPortTop) / m_framebufferHeight;
  float left = viewPortLeft * 2.0f / m_framebufferWidth - 1.0f;
  float bottom = (m_framebufferHeight - viewPortBottom) * 2.0f / m_framebufferHeight - 1.0f;
  float right = left + xScale * 2.0f;
  float top = bottom + yScale * 2.0f;
  float middleX = left + xScale;
  float middleY = bottom + yScale;

  drawGnomon(viewRotation * view, proj, xScale, yScale, middleX, middleY);
  drawGnomon(viewRotation * view * model, proj, xScale, yScale, middleX, middleY);


	setLineColour(vec3(1.0f, 1.0f, 1.0f));

  drawLine(vec2(left, top), vec2(right, top));
  drawLine(vec2(right, top), vec2(right, bottom));
  drawLine(vec2(right, bottom), vec2(left, bottom));
  drawLine(vec2(left, bottom), vec2(left, top));

  for (int i = 0; i < 24; i+=2) {
    vec4 A(vertices[edges[i] * 3], vertices[edges[i] * 3 + 1], vertices[edges[i] * 3 + 2], 1.0f);
    vec4 B(vertices[edges[i + 1] * 3], vertices[edges[i + 1] * 3 + 1], vertices[edges[i + 1] * 3 + 2], 1.0f);

    A = viewRotation * view * model * modelScale * A;
    B = viewRotation * view * model * modelScale * B;


    bool draw = clip(A, B, 0); //near-field clip
    if (!draw) continue;


    A = proj * A;
    B = proj * B;
    divideByW(A);
    divideByW(B);

    for (int c = 2; c < 12; c += 2) {
      if (!clip(A, B, c)) {
        draw = false;
        break;
      }
    }

    if (draw) {
      drawLine(
        vec2(A.x * xScale + middleX, A.y * yScale + middleY),
        vec2(B.x * xScale + middleX, B.y * yScale + middleY));
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

    if( ImGui::Button( "Reset" ) ) {
      reset();
    }

    ImGui::RadioButton( "Rotate View", (int*)&mode, ROTATE_VIEW );
    ImGui::RadioButton( "Translate View", (int*)&mode, TRANSLATE_VIEW );
    ImGui::RadioButton( "Perspective", (int*)&mode, PERSPECTIVE );
    ImGui::RadioButton( "Rotate Model", (int*)&mode, ROTATE_MODEL );
    ImGui::RadioButton( "Translate Model", (int*)&mode, TRANSLATE_MODEL );
    ImGui::RadioButton( "Scale Model", (int*)&mode, SCALE_MODEL );
    ImGui::RadioButton( "Viewport", (int*)&mode, VIEWPORT );

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text( "Near field: %.1f", clippingPlanes[0].z );
		ImGui::Text( "Far field: %.1f", clippingPlanes[2].z );

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
          viewRotation = glm::rotate(viewRotation, 0.1f * xOffset * ROTATE_FACTOR, vec3(1, 0, 0));
        }
        if (middleDown) {
          viewRotation = glm::rotate(viewRotation, 0.1f * xOffset * ROTATE_FACTOR, vec3(0, 1, 0));
        }
        if (rightDown) {
          viewRotation = glm::rotate(viewRotation, 0.1f * xOffset * ROTATE_FACTOR, vec3(0, 0, 1));
        }
        break;
      case(TRANSLATE_VIEW):
        if (leftDown) {
          view = glm::translate(view, vec3(0.1f * xOffset * TRANSLATE_FACTOR, 0, 0));
        }
        if (middleDown) {
          view = glm::translate(view, vec3(0, 0.1f * xOffset * TRANSLATE_FACTOR, 0));
        }
        if (rightDown) {
          view = glm::translate(view, vec3(0, 0, 0.1f * xOffset * TRANSLATE_FACTOR));
        }
        break;

      case(ROTATE_MODEL):
        if (leftDown) {
          model = glm::rotate(model, 0.1f * xOffset * ROTATE_FACTOR, vec3(1, 0, 0));
        }
        if (middleDown) {
          model = glm::rotate(model, 0.1f * xOffset * ROTATE_FACTOR, vec3(0, 1, 0));
        }
        if (rightDown) {
          model = glm::rotate(model, 0.1f * xOffset * ROTATE_FACTOR, vec3(0, 0, 1));
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
          // model = glm::scale(model, vec3(pow(1.1f, xOffset), 1.0f, 1.0f));
          modelScale = glm::scale(modelScale, vec3(pow(1.1f, xOffset * SCALE_FACTOR), 1.0f, 1.0f));

        }
        if (middleDown) {
          // model = glm::scale(model, vec3(1.0f, pow(1.1f, xOffset), 1.0f));
          modelScale = glm::scale(modelScale, vec3(1.0f, pow(1.1f, xOffset * SCALE_FACTOR), 1.0f));
        }
        if (rightDown) {
          // model = glm::scale(model, vec3(1.0f, 1.0f, pow(1.1f, xOffset)));
          modelScale = glm::scale(modelScale, vec3(1.0f, 1.0f, pow(1.1f, xOffset * SCALE_FACTOR)));
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
          viewPortX2 = xPos < m_framebufferWidth ? (xPos > 0 ? xPos : 0): m_framebufferWidth;
          viewPortY1 = yPos < m_framebufferHeight ? (yPos > 0 ? yPos : 0): m_framebufferHeight;
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
          viewPortX1 = mouseLastX;
          viewPortY2 = mouseLastY;
          viewPortX2 = mouseLastX;
          viewPortY1 = mouseLastY;
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
     case(GLFW_KEY_A):
      reset();
      eventHandled = true;
      break;
     case(GLFW_KEY_Q):
      glfwSetWindowShouldClose(m_window, GL_TRUE);
      eventHandled = true;
      break;
     }
  }

	// Fill in with event handling code...

	return eventHandled;
}
