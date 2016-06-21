#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "polyroots.hpp"
#include <iostream>
using namespace glm;

class Primitive {
public:
  virtual ~Primitive();
  virtual bool intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d) {
    return 0;
  }
  virtual bool intersect(const glm::vec3& eye, const glm::vec3& ray, glm::mat4 transform, glm::vec3& point, glm::vec3& normal, float& d) {
    return 0;
  }
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
};

class Cube : public Primitive {
public:
  virtual ~Cube();
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d) {
    double roots[2];
    size_t i = quadraticRoots(dot(ray, ray),
      2 * dot(ray, eye - m_pos),
      dot(eye - m_pos, eye - m_pos) - m_radius * m_radius,
      roots);
    switch(i) {
      case 0:
        return false;
      case 1:
        point = eye + (float)roots[0] * ray;
        normal = point - m_pos;
        d = (float)roots[0];
        return true;
      case 2:
        double root;
        if (roots[0] < 0 && roots[1] < 0) {
          root = roots[0] < roots[1] ? roots[1] : roots[0];
        } else if (roots[0] < 0) {
          root = roots[1];
        } else if (roots[1] < 0) {
          root = roots[0];
        } else {
          root = roots[0] > roots[1] ? roots[1] : roots[0];
        }
        point = eye + (float)root * ray;
        normal = normalize(point - m_pos);
        d = (float)root;
        return true;
    }
  }

private:

  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d);
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, const glm::mat4 transform, glm::vec3& point, glm::vec3& normal, float& d);

  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};
