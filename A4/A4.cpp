#include <glm/ext.hpp>

#include "A4.hpp"
#include "Primitive.hpp"
#include "polyroots.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"

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
  vec3 mainRay = normalize(view - eye);

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
      image(x, y, 0) = 0;
      image(x, y, 1) = 0;
      image(x, y, 2) = 0;
      double angley = ((double)y - (double)h / 2.0) / (double)h * fovy;
      double anglex = -((double)x - (double)w / 2.0) / (double)w * fovx;
      vec3 ray = rotate(mainRay, radians((float)anglex), up);
      ray = rotate(ray, radians((float)angley), cross(up, mainRay));
      for (SceneNode* node : root->children) {
        if (node->m_nodeType == NodeType::GeometryNode) {
					GeometryNode* geonode = (GeometryNode*)node;
          vec3 point;
          vec3 normal;
          size_t i = geonode->m_primitive->intersect(eye, ray, point, normal);
					if (i > 0) {
						// image(x, y, 0) = 1;
						vec3 colour;
						PhongMaterial* phong = (PhongMaterial*)geonode->m_material;
						for (Light* light : lights) {
							vec3 l = normalize(light->position - point);
							vec3 r = reflect(l, normal);
							for (int c = 0; c < 3; c++) {
								float dotp = dot(l, normal);
								if (dotp < 0) dotp = 0;
								colour[c] += phong->m_kd[c] * dotp * light->colour[c];

								dotp = dot(r, -ray);
								if (dotp < 0) dotp = 0;
								colour[c] += phong->m_ks[c] * pow(dotp, phong->shininess) * light->colour[c];
							}

						}


						for (int c = 0; c < 3; c++) {
							image(x, y, c) = colour[c];
						}
					}
        }
      }
		}
	}

}
