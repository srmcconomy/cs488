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
  virtual bool intersect(const glm::vec3& eye, const glm::vec3& ray, const glm::mat4& transform, glm::vec3& point, glm::vec3& normal, float& d) {
    return 0;
  }
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d);
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, const glm::mat4& transform, glm::vec3& point, glm::vec3& normal, float& d);
};

class Cube : public Primitive {
public:
  virtual ~Cube();
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d);
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, const glm::mat4& transform, glm::vec3& point, glm::vec3& normal, float& d);
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d);
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, const glm::mat4& transform, glm::vec3& point, glm::vec3& normal, float& d);


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
  bool intersect(const glm::vec3& eye, const glm::vec3& ray, const glm::mat4& transform, glm::vec3& point, glm::vec3& normal, float& d);

  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};
