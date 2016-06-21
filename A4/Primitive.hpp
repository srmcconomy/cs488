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
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d) {
    vec3 ns[6] = {
      vec3(0, 0, (float)m_size / 2.0f),
      vec3(0, 0, -(float)m_size / 2.0f),
      vec3(0, (float)m_size / 2.0f, 0),
      vec3(0, -(float)m_size / 2.0f, 0),
      vec3((float)m_size / 2.0f, 0, 0),
      vec3(-(float)m_size / 2.0f, 0, 0)
    };
    bool isect = false;
    for (uint i = 0; i < 6; i++) {
      vec3 n = ns[i];
      vec3 p = m_pos + vec3(m_size / 2.0f, m_size / 2.0f, m_size / 2.0f) + n;
      float d2 = dot(p - eye, n) / dot(ray, n);
      if (!isect || (d2 * d > 0 && abs(d2) < abs(d))) {
        vec3 pt = eye + ray * d2;
        if ((pt - p).x > -m_size / 2.0f && (pt - p).x < m_size / 2.0f
          && (pt - p).y > -m_size / 2.0f && (pt - p).y < m_size / 2.0f
          && (pt - p).z > -m_size / 2.0f && (pt - p).z < m_size / 2.0f) {
            d = d2;
            point = pt;
            normal = normalize(n);
            isect = true;
        }
      }
    }
    return isect;
  }

  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};
