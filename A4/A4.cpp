#include <glm/ext.hpp>
#include <stack>

#include "A4.hpp"
#include "Primitive.hpp"
#include "polyroots.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"

using namespace glm;
using namespace std;

#define EPSILON 0.001f

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
	std::cout << to_string(mainRay) << std::endl;

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
      image(x, y, 0) = 0;
      image(x, y, 1) = 0;
      image(x, y, 2) = 0;
      double angley = ((double)y - (double)h / 2.0) / (double)h * fovy;
      double anglex = -((double)x - (double)w / 2.0) / (double)w * fovx;
      vec3 ray = rotate(mainRay, radians((float)anglex), up);
      ray = rotate(ray, radians((float)angley), cross(up, mainRay));
      float mind;
      bool anyobj = false;
			stack<mat4> transStack;
			transStack.push(root->trans);
			// std::cout << to_string(root->trans) << std::endl;
      for (SceneNode* node : root->children) {
        if (node->m_nodeType == NodeType::GeometryNode) {
					GeometryNode* geonode = (GeometryNode*)node;
          vec3 point;
          vec3 normal;
          float d;
					mat4 trans = transStack.top() * node->trans;
					bool isect = geonode->m_primitive->intersect(eye, ray, trans, point, normal, d);
					if (isect && (!anyobj || d < mind)) {
            mind = d;
            anyobj = true;
						// image(x, y, 0) = 1;
						vec3 colour;
						PhongMaterial* phong = (PhongMaterial*)geonode->m_material;

						for (int c = 0; c < 3; c++) {
							colour[c] += phong->m_kd[c] * ambient[c];
						}

						for (Light* light : lights) {
							bool lightHits = true;
							vec3 l = normalize(point - light->position);
							vec3 point2;
							vec3 normal2;
							float dNode;
						  geonode->m_primitive->intersect(light->position, l, trans, point2, normal2, dNode);
              vec3 distance = point2 - point;
              std::cout << to_string(point) << to_string(point2) << std::endl;
              if (abs(dot(distance, distance)) > EPSILON) {
                lightHits = false;
              } else {
                for (SceneNode* node2 : root->children) {
  								if (node2->m_nodeType != NodeType::GeometryNode || node2->m_nodeId == node->m_nodeId) {
  									continue;
  								}
  								GeometryNode* geonode2 = (GeometryNode*)node2;

  			          float d2;

									mat4 trans2 = transStack.top() * node2->trans;
  								bool isect2 = geonode2->m_primitive->intersect(light->position, l, trans2, point2, normal2, d2);
  								if (isect2 && d2 * dNode > 0 && abs(d2) < abs(dNode)) {
  									lightHits = false;
  									break;
  								}
  							}
              }
							if (lightHits) {
                float attenuation = 1.0f / (light->falloff[0] + dNode * light->falloff[1] + dNode * dNode * light->falloff[2]);
								vec3 r = reflect(l, normal);
								for (int c = 0; c < 3; c++) {
									float dotp = dot(-l, normal);
									if (dotp < 0) dotp = 0;
									colour[c] += phong->m_kd[c] * dotp * light->colour[c] * attenuation;

									dotp = dot(r, -ray);
									if (dotp < 0) dotp = 0;
									colour[c] += phong->m_ks[c] * pow(dotp, phong->m_shininess) * light->colour[c] * attenuation;
								}
							}

						}


						for (int c = 0; c < 3; c++) {
							image(x, y, c) = colour[c] > 1.0f ? 1.0f : colour[c];
						}
					}
        }
      }
      if (!anyobj) {
        image(x, y, 2) = (float)y / (float)h;
      }
		}
	}

}
