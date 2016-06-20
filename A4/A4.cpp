#include <glm/ext.hpp>

#include "A4.hpp"
#include "Primitive.hpp"
#include "polyroots.hpp"
#include "GeometryNode.hpp"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...

  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

  double fovx = fovy / h * w;
  vec3 mainRay = view - eye;
  vec4 mainRay4(mainRay.x, mainRay.y, mainRay.z, 0);
  vec4 up4(up.x, up.y, up.z, 0);

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
      image(x, y, 0) = 0;
      image(x, y, 1) = 0;
      image(x, y, 2) = 0;
      double angley = ((double)y - (double)h / 2.0) / (double)h * fovy;
      double anglex = ((double)x - (double)w / 2.0) / (double)w * fovx;
      cout << "X: " << anglex << endl;
      cout << "Y: " << angley << endl;
      vec3 ray = rotate(mainRay, (float)anglex, up);
      ray = rotate(ray, (float)angley, cross(up, mainRay));
      for (SceneNode* node : root->children) {
        if (node->m_nodeType == NodeType::GeometryNode) {
          NonhierSphere* sphere = (NonhierSphere*)((GeometryNode*)node)->m_primitive;
          double roots[2];
          size_t intersect = quadraticRoots(dot(ray, ray),
            2 * dot(ray, eye - sphere->m_pos),
            dot(eye - sphere->m_pos, eye - sphere->m_pos) - sphere->m_radius * sphere->m_radius,
            roots);
          if (intersect > 0) {
            image(x, y, 0) = 1.0;
          }
        }
      }
		}
	}

}
