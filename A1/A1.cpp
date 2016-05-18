#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;
static const size_t NUM_COLOURS = 8;
static const float ROTATE_FACTOR = 0.01f;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1() : current_col(0), rotation(0), scale(1.0f), mouseLastX(0), mouseDown(false) {
  colours = new float[NUM_COLOURS][3];
  for (int i = 0; i < NUM_COLOURS; i++) {
    colours[i][0] = 0;
    colours[i][1] = 0;
    colours[i][2] = 0;
  }
  heights = new int[DIM * DIM];
  cubeColours = new int[DIM * DIM];
  for (int x = 0; x < DIM; x++) {
    for (int y = 0; y < DIM; y++) {
      heights[x * DIM + y] = 0;
      cubeColours[x * DIM + y] = 0;
    }
  }
  currentPos[0] = 0;
  currentPos[1] = 0;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1() {
  delete [] colours;
  delete [] heights;
  delete [] cubeColours;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init() {

  // Set the background colour.
  glClearColor( 0.3, 0.5, 0.7, 1.0 );

  // Build the shader
  m_shader.generateProgramObject();
  m_shader.attachVertexShader(
    getAssetFilePath( "VertexShader.vs" ).c_str() );
  m_shader.attachFragmentShader(
    getAssetFilePath( "FragmentShader.fs" ).c_str() );
  m_shader.link();

  // Set up the uniforms
  P_uni = m_shader.getUniformLocation( "P" );
  V_uni = m_shader.getUniformLocation( "V" );
  M_uni = m_shader.getUniformLocation( "M" );
  col_uni = m_shader.getUniformLocation( "colour" );

  initGrid();
  initCube();

  // Set up initial view and projection matrices (need to do this here,
  // since it depends on the GLFW window being set up correctly).
  view = glm::lookAt(
    glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
    glm::vec3( 0.0f, 0.0f, 0.0f ),
    glm::vec3( 0.0f, 1.0f, 0.0f ) );
  proj = glm::perspective(
    glm::radians( 45.0f ),
    float( m_framebufferWidth ) / float( m_framebufferHeight ),
    1.0f, 1000.0f );
}

void A1::initGrid() {
  size_t sz = 3 * 2 * 2 * (DIM+3);

  float *verts = new float[ sz ];
  size_t ct = 0;
  for( int idx = 0; idx < DIM+3; ++idx ) {
    verts[ ct ] = -1;
    verts[ ct+1 ] = 0;
    verts[ ct+2 ] = idx-1;
    verts[ ct+3 ] = DIM+1;
    verts[ ct+4 ] = 0;
    verts[ ct+5 ] = idx-1;
    ct += 6;

    verts[ ct ] = idx-1;
    verts[ ct+1 ] = 0;
    verts[ ct+2 ] = -1;
    verts[ ct+3 ] = idx-1;
    verts[ ct+4 ] = 0;
    verts[ ct+5 ] = DIM+1;
    ct += 6;
  }

  // Create the vertex array to record buffer assignments.
  glGenVertexArrays( 1, &m_grid_vao );
  glBindVertexArray( m_grid_vao );

  // Create the cube vertex buffer
  glGenBuffers( 1, &m_grid_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
  glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
    verts, GL_STATIC_DRAW );

  // Specify the means of extracting the position values properly.
  GLint posAttrib = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray( posAttrib );
  glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

  // Reset state to prevent rogue code from messing with *my*
  // stuff!
  glBindVertexArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  // OpenGL has the buffer now, there's no need for us to keep a copy.
  delete [] verts;

  CHECK_GL_ERRORS;
}

void A1::initCube() {
  float vertices[] = {
    //bottom
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    //top
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f
  };
  unsigned short indices[] = {
    //bottom
    0, 1, 2,
    3, 1, 2,
    //left
    0, 1, 4,
    5, 1, 4,
    //front
    1, 3, 5,
    7, 3, 5,
    //right
    3, 2, 7,
    6, 2, 7,
    //back
    0, 2, 4,
    6, 2, 4,
    //top
    4, 5, 6,
    7, 5, 6
  };

  glGenVertexArrays(1, &m_cube_vao);
  glBindVertexArray(m_cube_vao);

  glGenBuffers(2, &m_cube_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);

  GLint posAttrib = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray( posAttrib );
  glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

  glGenBuffers(1, &m_cube_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cube_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned short),
    indices, GL_STATIC_DRAW);

  glBindVertexArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  CHECK_GL_ERRORS;
}

void A1::initSquare() {
  float vertices[] = {
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f
  };
  unsigned short indices[] = {
    0, 1, 2, 3
  };

  glGenVertexArrays(1, &m_square_vao);
  glBindVertexArray(m_square_vao);

  glGenBuffers(2, &m_square_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, m_square_vbo);
  glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);

  GLint posAttrib = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray( posAttrib );
  glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

  glGenBuffers(1, &m_square_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_square_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(unsigned short),
    indices, GL_STATIC_DRAW);

  glBindVertexArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  CHECK_GL_ERRORS;
}



//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic() {
  // Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic() {
  // We already know there's only going to be one window, so for
  // simplicity we'll store button states in static local variables.
  // If there was ever a possibility of having multiple instances of
  // A1 running simultaneously, this would break; you'd want to make
  // this into instance fields of A1.
  static bool showTestWindow(false);
  static bool showDebugWindow(true);

  ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
  float opacity(0.5f);

  ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
    if( ImGui::Button( "Quit Application" ) ) {
      glfwSetWindowShouldClose(m_window, GL_TRUE);
    }

    //All Colour controls
    for (int i = 0; i < NUM_COLOURS; i++) {
      ImGui::PushID( i );
      ImGui::ColorEdit3( "##Colour", colours[i] );
      ImGui::SameLine();
      if( ImGui::RadioButton( "##Col", &current_col, i ) ) {
          cubeColours[currentPos[0] * DIM + currentPos[1]] = current_col;
      }
      ImGui::PopID();
    }

/*
    // For convenience, you can uncomment this to show ImGui's massive
    // demonstration window right in your application.  Very handy for
    // browsing around to get the widget you want.  Then look in
    // shared/imgui/imgui_demo.cpp to see how it's done.
    if( ImGui::Button( "Test Window" ) ) {
      showTestWindow = !showTestWindow;
    }
*/

    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
    ImGui::Text( "X: %d", currentPos[0] );
    ImGui::Text( "Y: %d", currentPos[1] );

  ImGui::End();

  if( showTestWindow ) {
    ImGui::ShowTestWindow( &showTestWindow );
  }
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw() {
  // Create a global transformation for the model (centre it).
  mat4 W;
  W = glm::scale(W, vec3(scale, scale, scale));
  W = glm::rotate(W, rotation, vec3(0, 1, 0));
  mat4 M;
  M = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

  m_shader.enable();
    glEnable( GL_DEPTH_TEST );

    glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
    glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
    glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( M ) );
    glUniform3f( col_uni, 1, 1, 1 );

    glBindVertexArray( m_grid_vao );
    glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

    M = glm::translate( W, vec3( -7.5f, 0.5f, -7.5f ) );
    M = glm::scale(M, vec3(0.5f, 0.5f, 0.5f));

    glBindVertexArray(m_cube_vao );

    for (int x = 0; x < DIM; x++) {
      for (int y = 0; y < DIM; y++) {
        float* colour = colours[cubeColours[x * DIM + y]];
        glUniform3f( col_uni, colour[0], colour[1], colour[2] );
        mat4 M2 = glm::translate(M, vec3((float)x * 2.0f, 0, (float)y * 2.0f));
        for (int h = 0; h < heights[x * DIM + y]; h++) {
          glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( M2 ) );
          glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0 );
          M2 = glm::translate(M2, vec3(0, 2.0f, 0));
        }
      }
    }

    glBindVertexArray(m_square_vao);
    M = glm::translate(M, vec3((float)currentPos[0] * 2.0f, (float)heights[currentPos[0] * DIM + currentPos[1]] * 2.0f, (float)currentPos[1] * 2.0f));

  m_shader.disable();

  // Restore defaults
  glBindVertexArray( 0 );

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup() { }

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent(int entered) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A1::mouseMoveEvent(double xPos, double yPos) {
  bool eventHandled(false);

  if (!ImGui::IsMouseHoveringAnyWindow()) {
      if (mouseDown) {
        rotation += (xPos - mouseLastX) * ROTATE_FACTOR;
      }
      mouseLastX = xPos;
  }

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int action, int mods) {
  bool eventHandled(false);

  if (!ImGui::IsMouseHoveringAnyWindow()) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
      if (action == GLFW_PRESS) {
        mouseDown = true;
      }
      if (action == GLFW_RELEASE) {
        mouseDown = false;
      }

    // The user clicked in the window.  If it's the left
    // mouse button, initiate a rotation.
  }

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
  bool eventHandled(false);

  scale *= pow(1.2, yOffSet);

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
  bool eventHandled(false);

  // Fill in with event handling code...
  if( action == GLFW_PRESS ) {
    switch(key) {
     case(GLFW_KEY_SPACE):
      if (heights[currentPos[0] * DIM + currentPos[1]] == 0) cubeColours[currentPos[0] * DIM + currentPos[1]] = current_col;
      heights[currentPos[0] * DIM + currentPos[1]]++;
      eventHandled = true;
      break;
     case(GLFW_KEY_BACKSPACE):
      if (heights[currentPos[0] * DIM + currentPos[1]] > 0) heights[currentPos[0] * DIM + currentPos[1]]--;
      eventHandled = true;
      break;
     case(GLFW_KEY_LEFT):
      moveCursor(-1, 0, mods & GLFW_MOD_SHIFT);
      eventHandled = true;
      break;
     case(GLFW_KEY_RIGHT):
      moveCursor(1, 0, mods & GLFW_MOD_SHIFT);
      eventHandled = true;
      break;
     case(GLFW_KEY_UP):
      moveCursor(0, -1, mods & GLFW_MOD_SHIFT);
      eventHandled = true;
      break;
     case(GLFW_KEY_DOWN):
      moveCursor(0, 1, mods & GLFW_MOD_SHIFT);
      eventHandled = true;
      break;
     case(GLFW_KEY_R):
      reset();
      break;
     case(GLFW_KEY_Q):
      glfwSetWindowShouldClose(m_window, GL_TRUE);
      break;
    }
  }
  return eventHandled;
}

void A1::moveCursor(int x, int y, bool copy) {
  if (currentPos[0] + x < 0 || currentPos[0] + x > DIM - 1 ||
    currentPos[1] + y < 0 || currentPos[1] + y > DIM - 1)
    return;
  int h = heights[currentPos[0] * DIM + currentPos[1]];
  int c = cubeColours[currentPos[0] * DIM + currentPos[1]];
  currentPos[0] += x;
  currentPos[1] += y;
  if (copy) {
    heights[currentPos[0] * DIM + currentPos[1]] = h;
    cubeColours[currentPos[0] * DIM + currentPos[1]] = c;
  }
}

void A1::reset() {
  currentPos[0] = 0;
  currentPos[1] = 0;
  for (int x = 0; x < DIM; x++) {
    for (int y = 0; y < DIM; y++) {
      heights[x * DIM + y] = 0;
      cubeColours[x * DIM + y] = 0;
    }
  }
  for (int i = 0; i < NUM_COLOURS; i++) {
    colours[i][0] = 0;
    colours[i][1] = 0;
    colours[i][2] = 0;
  }
  rotation = 0;
  scale = 1.0f;
  current_col = 0;

}
