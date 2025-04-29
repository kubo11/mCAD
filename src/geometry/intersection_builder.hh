#ifndef MCAD_INTERSECTION_BUILDER
#define MCAD_INTERSECTION_BUILDER

#include "mge.hh"

class IntersectionBuilder {
 public:
  IntersectionBuilder() = default;
  ~IntersectionBuilder() = default;

  static std::pair<glm::vec2, glm::vec2> find_starting_point(mge::Entity& s1, mge::Entity& s2, std::optional<glm::vec3> beg = {});
  static std::pair<std::vector<glm::vec2>, std::vector<glm::vec2>> find(mge::Entity& s1, mge::Entity& s2, const glm::vec2& uv, const glm::vec2& st, float newton_factor, float max_dist, bool rough);

 private:
  static glm::vec3 get_random_beg(mge::Entity& s1, mge::Entity& s2);
  static glm::vec2 get_closest_point_on_surface(mge::Entity& s, const glm::vec3& p);
  static glm::vec2 get_closest_point_on_surface_with_penalty(mge::Entity& s, const glm::vec3& p, const glm::vec2& penalizing_point);
  static std::pair<glm::vec2, glm::vec2> conjugate_gradient(mge::Entity& s1, mge::Entity& s2, const glm::vec2& uv, const glm::vec2& st);
};

#endif // MCAD_INTERSECTION_BUILDER