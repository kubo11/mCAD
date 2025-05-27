#ifndef MCAD_INTERSECTION_BUILDER
#define MCAD_INTERSECTION_BUILDER

#include "mge.hh"

class IntersectionBuilder {
 public:
  static constexpr int subd_outer = 40, subd_inner = 40;
  static constexpr float subd_accuracy = 0.0001, same_penalty_max = 60, same_penalty_len = 0.2;
  static constexpr int conj_grad_max_iter = 1500, conj_grad_max_iter_slow_error_change = 50;
  static constexpr float conj_grad_step = 0.01, conj_grad_accuracy = 0.0000033;
  static constexpr int find_newton_iters = 150, find_max_points = 1e5;
  static constexpr float find_max_pq_distance_in_self = 0.1, find_min_uv_st_distance_in_self = 0.01,
          find_min_improvement = 0.001, find_min_step = 0.0001, infinity =
                  std::numeric_limits<float>::infinity(), find_dist = 0.05;

  IntersectionBuilder() = default;
  ~IntersectionBuilder() = default;

  static std::pair<glm::vec2, glm::vec2> find_starting_point(mge::Entity& s1, mge::Entity& s2, std::optional<glm::vec3> beg = std::nullopt);
  static std::pair<std::vector<glm::vec2>, std::vector<glm::vec2>> find(mge::Entity& s1, mge::Entity& s2, const glm::vec2& uv, const glm::vec2& st, float newton_factor, float max_dist, bool rough);
  static glm::vec3 get_surface_position(mge::Entity& s, const glm::vec2& uv);

 private:
  static glm::vec3 get_random_beg(mge::Entity& s1, mge::Entity& s2);
  static glm::vec2 get_closest_point_on_surface(mge::Entity& s, const glm::vec3& p);
  static glm::vec2 get_closest_point_on_surface_with_penalty(mge::Entity& s, const glm::vec3& p, const glm::vec2& penalizing_point);
  static std::pair<glm::vec2, glm::vec2> conjugate_gradient(mge::Entity& s1, mge::Entity& s2, glm::vec2 uv, glm::vec2 st);
  static std::pair<glm::vec3, glm::vec3> get_surface_gradient(mge::Entity& s, const glm::vec2& uv);
  static float penalty_function(glm::vec2 uv, glm::vec2 st);
  static glm::vec2 normalize_parametrization(mge::Entity& s, glm::vec2 uv);
  static bool are_parameters_normalized(mge::Entity& s, glm::vec2 uv);
  static glm::vec2 clamp_parameters(mge::Entity& s, glm::vec2 uv);
  static std::pair<glm::vec2, glm::vec2> get_gradients(mge::Entity& s1, mge::Entity& s2, glm::vec2 uv, glm::vec2 st);
};

#endif // MCAD_INTERSECTION_BUILDER