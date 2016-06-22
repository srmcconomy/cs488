#include <glm/ext.hpp>

#include "A4.hpp"
#include "Primitive.hpp"
#include "polyroots.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"

using namespace glm;
using namespace std;

#define EPSILON 0.01f

void lighting(SceneNode* root, SceneNode* node, mat4 trans, Light* light, vec3 l, float dNode, bool& lightHits) {
  mat4 nodetrans = trans * root->trans;
  if (root->m_nodeType == NodeType::GeometryNode && root->m_nodeId != node->m_nodeId) {
    GeometryNode* geonode2 = (GeometryNode*)root;

    float d2;
    vec3 point2;
    vec3 normal2;

    bool isect2 = geonode2->m_primitive->intersect(light->position, l, nodetrans, point2, normal2, d2);
    if (isect2 && d2 * dNode > 0 && abs(d2) < abs(dNode)) {
      lightHits = false;
      return;
    }
  }
  for (SceneNode* node2 : root->children) {
    lighting(node2, node, nodetrans, light, l, dNode, lightHits);
    if (!lightHits) return;
  }
}


void render(SceneNode* node, SceneNode* root, mat4 trans, vec3 eye, vec3 ray, vec3& colour, std::list<Light*> lights, vec3 ambient, float& mind, bool& anyobj) {
  mat4 nodetrans = trans * node->trans;
  if (node->m_nodeType == NodeType::GeometryNode) {
    GeometryNode* geonode = (GeometryNode*)node;
    vec3 point;
    vec3 normal;
    float d;
    bool isect = geonode->m_primitive->intersect(eye, ray, nodetrans, point, normal, d);
    if (isect && (!anyobj || d < mind)) {
      colour = vec3(0);
      mind = d;
      anyobj = true;
      // image(x, y, 0) = 1;
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
        geonode->m_primitive->intersect(light->position, l,nodetrans, point2, normal2, dNode);
        vec3 distance = point2 - point;
        // std::cout << to_string(point) << to_string(point2) << std::endl;
        if (abs(dot(distance, distance)) > EPSILON) {
          lightHits = false;
        } else {
          lighting(root, node, mat4(1.0f), light, l, dNode, lightHits);
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
    }
  }
  for (SceneNode* n : node->children) {
    render(n, root, nodetrans, eye, ray, colour, lights, ambient, mind, anyobj);
  }
}


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
      float mind;
      bool anyobj = false;
      vec3 colour;
      render(root, root, mat4(1.0f), eye, ray, colour, lights, ambient, mind, anyobj);

      if (anyobj) {
        for (int c = 0; c < 3; c++) {
          image(x, y, c) = colour[c] > 1.0f ? 1.0f : colour[c];
        }
      } else {
        image(x, y, 0) = (float)y / (float)h / 2.0f;
        image(x, y, 2) = (float)y / (float)h;
      }
		}
    if ((y + 1) * 10 % h == 0) std::cout << (float)(y + 1)/ (float)h * 100.0f << '%' << std::endl;
	}

}
