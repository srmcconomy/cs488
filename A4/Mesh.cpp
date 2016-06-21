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

bool Mesh::intersect(const vec3& ray, const vec3& eye, vec3& point, vec3& normal, float& d) {
	bool isect = false;
	for (Triangle face : m_faces) {
		vec3 u = m_vertices[face.v1] - m_vertices[face.v2];
		vec3 v = m_vertices[face.v3] - m_vertices[face.v2];
		vec3 n = normalize(cross(u, v));
		float rayn = dot(ray, n);
		if (rayn >= 0) continue;
		float d2 = -dot(m_vertices[face.v2] - eye, n) / rayn;
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
			if (t < 0.0f || s + t > 1.0f) {
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
