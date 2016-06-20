#pragma once

#include <glm/glm.hpp>
#include "polyroots.hpp"
#include <iostream>

class Primitive {
public:
  virtual ~Primitive();
  virtual size_t intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal) {
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
  size_t intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal) {
    double roots[2];
    size_t i = quadraticRoots(dot(ray, ray),
      2 * dot(ray, eye - m_pos),
      dot(eye - m_pos, eye - m_pos) - m_radius * m_radius,
      roots);
    point = eye + (float)roots[0] * ray;
    normal = point - m_pos;
    std::cout << i << std::endl;
    return i;
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
  size_t intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal) {
    vec3 n = vec3(size, 0, 0);
    vec3 p = m_pos + n;
    double d = dot(p - origin, n) / dot(ray, n);
    point = eye + ray * d;
    if ((point - p).x > -m_pos && (point - p).x < m_pos) {
      return 1;
    }
    return 0;
  }

  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};
