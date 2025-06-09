#ifndef MCAD_INTERSECTION_BUILDER
#define MCAD_INTERSECTION_BUILDER

#include "mge.hh"

class IntersectionBuilder {
 public:
  static constexpr int s_surface_subdivision_count = 40, s_surface_subdivision_resolution = 80;
  static constexpr float s_surface_closest_point_accuracy = 0.0001, s_max_penalty = 60, s_penalty_length = 0.2, s_min_parameter_dist= 0.02f;

  static constexpr int s_conj_grad_max_iter = 1500, s_conj_grad_max_scarce_iter_count = 50;
  static constexpr float s_conj_grad_step = 0.01, s_conj_grad_accuracy = 0.0000033;

  static constexpr int s_newton_iters = 150, s_newton_max_points = 1e5;
  static constexpr float s_newton_min_scene_dist_for_self = 0.1, s_newton_min_param_dist_for_self = 0.01,
          s_newton_min_improvement = 0.001, s_newton_min_step = 0.0001, s_infinity = std::numeric_limits<float>::infinity(), find_dist = 0.05;

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