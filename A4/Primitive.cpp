#include "Primitive.hpp"

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

bool Sphere::intersect(const vec3& eye, const vec3& ray, const mat4& transform, vec3& point, vec3& normal, float& d) {
  mat4 inv = inverse(transform);
  vec4 eye4(eye.x, eye.y, eye.z, 1.0f);
  vec4 ray4(eye.x, eye.y, eye.z, 0);
  eye4 = inv * eye4;
  ray4 = inv * ray4;
  bool ret = intersect(vec3(eye4.y, eye4.x, eye4.z), vec3(ray4.x, ray4.y, ray4.z), point, normal, d);
  vec4 point4 = transform * vec4(point.x, point.y, point.z, 1.0f);
  vec4 normal4 = transform * vec4(normal.x, normal.y, normal.z, 0);
  point = vec3(point4.x, point4.y, point4.z);
  normal = vec3(normal4.x, normal4.y, normal4.z);
  d = length(point - eye);
  return ret;
}

bool Sphere::intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d) {
  double roots[2];
  size_t i = quadraticRoots(dot(ray, ray),
    2 * dot(ray, eye),
    dot(eye, eye) - 1.0f,
    roots);
  switch(i) {
    case 0:
      return false;
    case 1:
      point = eye + (float)roots[0] * ray;
      normal = point;
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
      normal = point;
      d = (float)root;
      return true;
  }
}

Cube::~Cube()
{
}

bool Cube::intersect(const vec3& eye, const vec3& ray, const mat4& transform, vec3& point, vec3& normal, float& d) {
  mat4 inv = inverse(transform);
  vec4 eye4(eye.x, eye.y, eye.z, 1.0f);
  vec4 ray4(eye.x, eye.y, eye.z, 0);
  eye4 = inv * eye4;
  ray4 = inv * ray4;
  bool ret = intersect(vec3(eye4.y, eye4.x, eye4.z), vec3(ray4.x, ray4.y, ray4.z), point, normal, d);
  point = transform * point;
  normal = transform * normal;
  d = length(point - eye);
  return ret;
}

bool Cube::intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d) {
  vec3 ns[6] = {
    vec3(0, 0, 1.0f),
    vec3(0, 0, -1.0f),
    vec3(0, 1.0f, 0),
    vec3(0, -1.0f, 0),
    vec3(1.0f, 0, 0),
    vec3(-1.0f, 0, 0)
  };
  bool isect = false;
  for (uint i = 0; i < 6; i++) {
    vec3 n = ns[i];
    vec3 p = vec3(0.5f, 0.5f, 0.5f) + n * 0.5f;
    float d2 = dot(p - eye, n) / dot(ray, n);
    if (!isect || (d2 * d > 0 && abs(d2) < abs(d))) {
      vec3 pt = eye + ray * d2;
      if ((pt - p).x > -0.5f && (pt - p).x < 0.5f
        && (pt - p).y > -0.5f && (pt - p).y < 0.5f
        && (pt - p).z > -0.5f && (pt - p).z < 0.5f) {
          d = d2;
          point = pt;
          normal = normalize(n);
          isect = true;
      }
    }
  }
  return isect;
}

NonhierSphere::~NonhierSphere()
{
}

bool NonhierSphere::intersect(const vec3& eye, const vec3& ray, const mat4& transform, vec3& point, vec3& normal, float& d) {
  return intersect(eye, ray, point, normal, d);
}

bool NonhierSphere::intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d) {
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

NonhierBox::~NonhierBox()
{
}

bool NonhierBox::intersect(const vec3& eye, const vec3& ray, const mat4& transform, vec3& point, vec3& normal, float& d) {
  return intersect(eye, ray, point, normal, d);
}

bool NonhierBox::intersect(const glm::vec3& eye, const glm::vec3& ray, glm::vec3& point, glm::vec3& normal, float& d) {
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
