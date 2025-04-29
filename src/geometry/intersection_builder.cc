#include "intersection_builder.hh"

std::pair<glm::vec2, glm::vec2> IntersectionBuilder::find_starting_point(mge::Entity& s1, mge::Entity& s2, std::optional<glm::vec3> beg = {}) {
  auto beg_point = beg.value_or(get_random_beg(s1, s2));
  auto uv = get_closest_point_on_surface(s1, beg_point);
  auto st = (s1 == s2) ? get_closest_point_on_surface_with_penalty(s2, beg_point, uv) : get_closest_point_on_surface(s2, beg_point);
  return conjugate_gradient(s1, s2, uv, st);
}

std::pair<std::vector<glm::vec2>, std::vector<glm::vec2>> IntersectionBuilder::find(mge::Entity& s1, mge::Entity& s2, const glm::vec2& uv, const glm::vec2& st, float newton_factor, float max_dist, bool rough) {

}

glm::vec3 IntersectionBuilder::get_random_beg(mge::Entity& s1, mge::Entity& s2) {
  std::srand(std::time(nullptr));
  auto random_factor = glm::normalize(glm::vec3(
    std::rand() / (float) RAND_MAX,
    std::rand() / (float) RAND_MAX,
    std::rand() / (float) RAND_MAX
  ));
  return (s1.get_component<mge::TransformComponent>().get_position() + s2.get_component<mge::TransformComponent>().get_position() + random_factor) * 0.5f;
}

glm::vec2 IntersectionBuilder::get_closest_point_on_surface(mge::Entity& s, const glm::vec3& p) {

}

glm::vec2 IntersectionBuilder::get_closest_point_on_surface_with_penalty(mge::Entity& s, const glm::vec3& p, const glm::vec2& penalizing_point) {

}

std::pair<glm::vec2, glm::vec2> IntersectionBuilder::conjugate_gradient(mge::Entity& s1, mge::Entity& s2, const glm::vec2& uv, const glm::vec2& st) {

}
