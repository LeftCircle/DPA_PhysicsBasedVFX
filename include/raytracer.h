#pragma once

#include <vector>
#include <optional>

namespace lux
{


template <typename Vec3>
Vec3 cross(const Vec3& v1, const Vec3& v2){
    return Vec3(
        v1.y() * v2.z() - v1.z() * v2.y(),
        v1.z() * v2.x() - v1.x() * v2.z(),
        v1.x() * v2.y() - v1.y() * v2.x()
    );
}

template <typename Vec3>
auto dot(const Vec3& v1, const Vec3& v2){
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}


// template<typename Vec, typename Vec3i>
// Vec get_ray_triangle_intersection(
//     Vec& start_pos,
//     Vec& ray_dir,
//     std::vector<Vec3i>& tris,
//     std::vector<Vec>& verts,
//     size_t idx
// ){
//     const Vec3i& tri = tris[idx]; 
//     Vec e1 = verts[tri[1]] - verts[tri[0]];
//     Vec e2 = verts[tri[2]] - verts[tri[1]];

//     Vec3 norm = cross(e1, e2);
//     auto d = dot(norm, verts[tri[0]] - start_pos) / dot(norm, ray_dir);
//     vec3 e2xe1 = cross(e2, e1);
//     auto num_part = start_pos - verts[tri[0]] + d * ray_dir;
//     auto e2xe1_magsq = dot(e2xe1, e2xe1);
//     auto u = dot(corss(e2, num_part), e2xe1) / e2xe1_magsq;
//     auto v = -(dot(cross(e1, num_part), e2xe1) / e2xe1_magsq);
//     return 
// };


template<typename Vec, typename Vec3i>
std::optional<Vec> get_ray_triangle_intersection(
    const Vec& start_pos,
    const Vec& ray_dir,
    const std::vector<Vec3i>& tris,
    const std::vector<Vec>& verts,
    std::size_t idx
){
    const Vec3i& tri = tris[idx];
    const Vec& v0 = verts[tri[0]];
    const Vec& v1 = verts[tri[1]];
    const Vec& v2 = verts[tri[2]];

    const Vec e1 = v1 - v0;
    const Vec e2 = v2 - v0;
    const Vec normal = cross(e1, e2);

    const auto denominator = dot(normal, ray_dir);

    // The ray is parallel to the triangle.
    if (std::abs(denominator) < 1e-6) {
        return std::nullopt;
    }

    const auto distance =
        dot(normal, v0 - start_pos) / denominator;

    // The intersection is behind the ray origin.
    if (distance < 0) {
        return std::nullopt;
    }

    const Vec hit = start_pos + distance * ray_dir;
    const Vec offset = hit - v0;

    const auto e1e1 = dot(e1, e1);
    const auto e1e2 = dot(e1, e2);
    const auto e2e2 = dot(e2, e2);
    const auto offset_e1 = dot(offset, e1);
    const auto offset_e2 = dot(offset, e2);

    const auto barycentric_denominator = e1e1 * e2e2 - e1e2 * e1e2;

    const auto u = (e2e2 * offset_e1 - e1e2 * offset_e2) / barycentric_denominator;

    const auto v = (e1e1 * offset_e2 - e1e2 * offset_e1) / barycentric_denominator;

    if (u < 0 || v < 0 || u + v > 1) {
        return std::nullopt;
    }

    return hit;
}

template<typename Vec, typename Vec4i>
std::optional<Vec> get_ray_face_intersection(
    const Vec& start_pos,
    const Vec& ray_dir,
    const std::vector<Vec4i>& faces,
    const std::vector<Vec>& verts,
    std::size_t idx
){
    const Vec4i& face = faces[idx];
    const Vec& v0 = verts[face[0]];
    const Vec& v1 = verts[face[1]];
    const Vec& v2 = verts[face[2]];
    const Vec& v3 = verts[face[3]];

    // Check intersection with the first triangle (v0, v1, v2)
    auto hit = get_ray_triangle_intersection(start_pos, ray_dir, std::vector<Vec4i>{Vec4i(0, 1, 2, 0)}, verts, 0);
    if (hit) {
        return hit;
    }

    // Check intersection with the second triangle (v0, v2, v3)
    hit = get_ray_triangle_intersection(start_pos, ray_dir, std::vector<Vec4i>{Vec4i(0, 2, 3, 0)}, verts, 0);
    if (hit) {
        return hit;
    }

    return std::nullopt;
};




    
} // namespace lux
