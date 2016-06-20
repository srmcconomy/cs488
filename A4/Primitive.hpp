#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "polyroots.hpp"
#include <iostream>
using namespace glm;

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
    return 0;
    double roots[2];
    size_t i = quadraticRoots(dot(ray, ray),
      2 * dot(ray, eye - m_pos),
      dot(eye - m_pos, eye - m_pos) - m_radius * m_radius,
      roots);
    point = eye + (float)roots[0] * ray;
    normal = point - m_pos;
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
    vec3 n[6] = {
      vec3(0, 0, (float)m_size),
      vec3(0, 0, -(float)m_size),
      vec3(0, (float)m_size, 0),
      vec3(0, -(float)m_size, 0),
      vec3((float)m_size, 0, 0),
      vec3(-(float)m_size, 0, 0)
    };
    bool isect = true;
    for (uint i = 0; i < 6; i++) {
      vec3 n n[i];
      vec3 p = m_pos + n;
      float d = dot(p - eye, n) / dot(ray, n);
      std::cout << "d " << d << std::endl;
      point = eye + ray * d;
      if ((point - p).x > -m_size && (point - p).x < m_size
        && (point - p).y > -m_size && (point - p).y < m_size) {
      } else {
        isect = false;
      }
    }
    return isect ? 1 : 0;
  }

  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};
