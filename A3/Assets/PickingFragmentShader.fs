#version 330

out vec4 color;

uniform vec4 PickingColor;

void main() {
  color = PickingColor;
}
