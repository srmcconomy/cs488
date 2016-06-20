#pragma once

#include <glm/glm.hpp>
#include "polyroots.hpp"

class Primitive {
public:
  virtual ~Primitive();
  virtual size_t intersect(const glm::vec3& eye, glm::vec3& ray, glm::vec3& point, glm::vec3& normal);
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
  size_t intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal) {
    double roots[2];
    size_t i = quadraticRoots(dot(ray, ray),
      2 * dot(ray, eye - m_pos),
      dot(eye - m_pos, eye - m_pos) - m_radius * m_radius,
      roots);
    point = eye + roots[0] * ray;
    normal = point - m_pos;
    return i;
  }

    glm::vec3 m_pos;
    double m_radius;

private:
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }

  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};
