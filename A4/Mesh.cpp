#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*

  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

bool Mesh::intersect(const vec3& eye, const vec3& ray, const mat4& transform, vec3& point, vec3& normal, float& d) {
  // mat4 inv = inverse(transform);
  vec4 eye4(eye.x, eye.y, eye.z, 1.0f);
  vec4 ray4(ray.x, ray.y, ray.z, 0);
  // eye4 = inv * eye4;
  // ray4 = inv * ray4;
  vec3 eye3 = vec3(eye4.y, eye4.x, eye4.z);
  vec3 ray3 = normalize(vec3(ray4.x, ray4.y, ray4.z));
  std::cout << to_string(eye) << to_string(eye3) << std::endl;
  std::cout << to_string(ray) << to_string(ray3) << std::endl;
  bool ret = intersect(eye3, ray3, point, normal, d);
  // vec4 point4 = transform * vec4(point.x, point.y, point.z, 1.0f);
  // mat3 sub = mat3(vec3(transform[0]), vec3(transform[1]), vec3(transform[2]));
  // sub = transpose(inverse(sub));
  // normal = normalize(sub * normal);
  // point = vec3(point4.x, point4.y, point4.z);
  // d = length(point - eye);
  return ret;

}

bool Mesh::intersect(const vec3& eye, const vec3& ray, vec3& point, vec3& normal, float& d) {
  bool isect = false;
	for (Triangle face : m_faces) {
		vec3 u = m_vertices[face.v1] - m_vertices[face.v2];
		vec3 v = m_vertices[face.v3] - m_vertices[face.v2];
		vec3 n = -normalize(cross(u, v));
		float nray = dot(ray, n);
		if (nray >= 0) continue;
		float d2 = dot(m_vertices[face.v2] - eye, n) / nray;

		if (d2 > 0 && (!isect || d2 < d)) {
			vec3 pt = eye + ray * d2;
			vec3 w = pt - m_vertices[face.v2];

			float uv = dot(u, v);
			float wv = dot(w, v);
			float vv = dot(v, v);
			float uu = dot(u, u);
			float wu = dot(w, u);
			float D = uv * uv - uu * vv;
			float s = (uv * wv - vv * wu) / D;
			if (s < 0.0f || s > 1.0f) {
				continue;
			}
			float t = (uv * wu - uu * wv) / D;
      // std::cout << t << std::endl;
      if (t < 0.0f || s + t  - 1.0f> 1.0f) {
				continue;
			}
			d = d2;
			point = pt;
			normal = n;
			isect = true;
		}
	}
	return isect;
}
