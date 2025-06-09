#include "intersection_builder.hh"

#include "components/bezier_surface_c0_component.hh"
#include "components/bezier_surface_c2_component.hh"
#include "components/torus_component.hh"

std::pair<glm::vec2, glm::vec2> IntersectionBuilder::find_starting_point(mge::Entity& s1, mge::Entity& s2, std::optional<glm::vec3> beg) {
  auto beg_point = beg.value_or(get_random_beg(s1, s2));
  auto uv = get_closest_point_on_surface(s1, beg_point);
  auto st = (s1 == s2) ? get_closest_point_on_surface_with_penalty(s2, beg_point, uv) : get_closest_point_on_surface(s2, beg_point);
  return conjugate_gradient(s1, s2, uv, st);
}

std::pair<std::vector<glm::vec2>, std::vector<glm::vec2>> IntersectionBuilder::find(mge::Entity& s1, mge::Entity& s2, const glm::vec2& uv, const glm::vec2& st, float newton_factor, float max_dist, bool rough) {
  bool self_intersection = s1 == s2;
  auto uv_first = uv, st_first = st;
  auto uv_now = uv, st_now = st;
  auto uv_newton = uv, st_newton = st;
  std::vector<glm::vec2> uv_vec = {uv_now};
  std::vector<glm::vec2> st_vec = {st_now};

  glm::vec3 t(0.0f, 0.0f, 0.0f);
  bool hit_edge = false;
  while (true) {
    float step = find_dist;
    float distance_now = s_infinity;
    glm::vec3 pos_r3 = get_surface_position(s1, uv_newton);
    bool out_of_bounds = false;
    auto g1 = get_surface_gradient(s1, uv_newton);
    auto g2 = get_surface_gradient(s2, st_newton);
    auto t_now = glm::normalize(glm::cross(glm::normalize(glm::cross(g1.first, g1.second)), glm::normalize(glm::cross(g2.first, g2.second))));

    if (hit_edge) {
      t_now *= -1.0f;
    }
    if (std::abs(glm::dot(glm::normalize(t_now), glm::normalize(t))) > 0.995) {
      t_now = t;
    }
    if (glm::dot(t, t_now) < 0) {
      t_now *= -1.0f;
    }

    for (int i = 0; i < s_newton_iters; ++i) {
      glm::vec3 p = get_surface_position(s1, uv_newton);
      glm::vec3 q = get_surface_position(s2, st_newton);
      auto grad_p = get_surface_gradient(s1, uv_newton);
      auto grad_q = get_surface_gradient(s2, st_newton);
      glm::vec4 free = glm::vec4(p - q, glm::dot(p - pos_r3, t_now) - step);
      float distance_newton = glm::length(free);
      if (distance_newton < max_dist) {
        uv_now = uv_newton;
        st_now = st_newton;
        break;
      }
      if (distance_newton > distance_now) {
        if (step <= s_newton_min_step) {
          if (hit_edge) {
            return {uv_vec, st_vec};
          }
          hit_edge = true;
          break;
        }
        step /= 2;
        distance_now = s_infinity;
        uv_newton = uv_now;
        st_newton = st_now;
        i = 0;
        continue;
      }
      distance_now = distance_newton;
      glm::mat4 jacobian = glm::mat4(
        {grad_p.first, glm::dot(grad_p.first, t_now)},
        {grad_p.second, glm::dot(grad_p.second, t_now)},
        {-grad_q.first, 0},
        {-grad_q.second, 0}
      );
      glm::vec4 uvst(uv_newton, st_newton);
      glm::vec4 duvst = glm::inverse(jacobian) * free;
      uvst = uvst - duvst * newton_factor;
      uv_newton = {uvst.x, uvst.y};
      st_newton = {uvst.z, uvst.w};
      out_of_bounds |= !are_parameters_normalized(s1, uv_newton) || !are_parameters_normalized(s2, st_newton);
      if (out_of_bounds || rough || glm::length(duvst) < s_newton_min_improvement) {
        uv_newton = clamp_parameters(s1, uv_newton);
        st_newton = clamp_parameters(s2, st_newton);
        break;
      }
    }
    if (!hit_edge) {
      uv_vec.emplace_back(uv_newton);
      st_vec.emplace_back(st_newton);
    } else {
      uv_vec.emplace(uv_vec.begin(), uv_newton);
      st_vec.emplace(st_vec.begin(), st_newton);
    }
    if (out_of_bounds || self_intersection && (
        glm::distance(uv_newton, st_newton) < s_newton_min_param_dist_for_self ||
        glm::distance(get_surface_position(s1, uv_newton), get_surface_position(s2, st_newton)) > s_newton_min_scene_dist_for_self)) {
      if (hit_edge) {
        return {uv_vec, st_vec};
      }
      uv_now = uv_first;
      st_now = st_first;
      uv_newton = uv_first;
      st_newton = st_first;
      hit_edge = true;
      t = {};
    } else {
      t = t_now;
    }

    if (!hit_edge && uv_vec.size() > 2 && glm::distance(get_surface_position(s1, uv_vec.front()), get_surface_position(s1, uv_vec.back()))
        < step) {
      return {uv_vec, st_vec};
    }

    if (uv_vec.size() > s_newton_max_points) {
      MGE_ERROR("Unable to find intersection for given parameters.");
      return {{}, {}};
    }
  }
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
  glm::vec2 delta(0.5, 0.5);
  glm::vec2 best(0.5, 0.5);
  float best_dist = glm::distance(get_surface_position(s, best), p);

  for (int i = 0; i < s_surface_subdivision_count; i++) {
    glm::vec2 start = best - delta;
    delta *= 2;
    delta /= s_surface_subdivision_resolution;
    for (int x = 0; x < s_surface_subdivision_resolution; x++) {
      float u = start.x + delta.x * x;
      if (u < 0 || u > 1) {
        continue;
      }
      for (int y = 0; y < s_surface_subdivision_resolution; y++) {
        float v = start.y + delta.y * y;
        if (v < 0 || v > 1) {
          continue;
        }
        glm::vec2 uv(u, v);

        float dist = glm::distance(get_surface_position(s, uv), p);
        if (dist < best_dist) {
          best_dist = dist;
          best = uv;
          if (best_dist < s_surface_closest_point_accuracy) {
            return best;
          }
        };
      }
    }
  }

  return best;
}

glm::vec2 IntersectionBuilder::get_closest_point_on_surface_with_penalty(mge::Entity& s, const glm::vec3& p, const glm::vec2& penalizing_point) {
  glm::vec2 delta(0.5, 0.5);
  glm::vec2 best(0.5, 0.5);
  float best_dist = glm::distance(get_surface_position(s, best), p) + penalty_function(best, penalizing_point);

  for (int i = 0; i < s_surface_subdivision_count; i++) {
    glm::vec2 start = best - delta;
    delta *= 2;
    delta /= s_surface_subdivision_resolution;
    for (int x = 0; x < s_surface_subdivision_resolution; x++) {
      float u = start.x + delta.x * x;
      if (u < 0 || u > 1) {
          continue;
      }
      for (int y = 0; y < s_surface_subdivision_resolution; y++) {
        float v = start.y + delta.y * y;
        if (v < 0 || v > 1) {
          continue;
        }
        glm::vec2 uv = normalize_parametrization(s, {u, v});

        if (glm::distance(uv, penalizing_point) < s_min_parameter_dist) continue;
        if (glm::distance(uv.x, penalizing_point.x) < s_min_parameter_dist) continue;
        if (glm::distance(uv.y, penalizing_point.y) < s_min_parameter_dist) continue;

        float dist = glm::distance(get_surface_position(s, uv), p) + penalty_function(uv, penalizing_point);
        if (dist < best_dist) {
          best_dist = dist;
          best = uv;
          if (best_dist < s_surface_closest_point_accuracy) {
            return best;
          }
        };
      }
    }
  }

  return best;
}

std::pair<glm::vec2, glm::vec2> IntersectionBuilder::conjugate_gradient(mge::Entity& s1, mge::Entity& s2, glm::vec2 uv, glm::vec2 st) {
  float dist = glm::distance(get_surface_position(s1, uv), get_surface_position(s2, st));
  float step = s_conj_grad_step;
  int scarce_iter_count = 0;

  for (int iter = 0; iter < s_conj_grad_max_iter; ++iter) {
    auto [grad_uv, grad_st] = get_gradients(s1, s2, uv, st);
    auto uv_new = uv - step * grad_uv;
    auto st_new = st - step * grad_st;
    float dist_new = glm::distance(get_surface_position(s1, uv_new), get_surface_position(s2, st_new));

    if (glm::distance(uv_new, uv) + glm::distance(st_new, st) < s_conj_grad_accuracy &&
      std::abs(dist - dist_new) < s_conj_grad_accuracy) {
      scarce_iter_count += 1;
      if (scarce_iter_count > s_conj_grad_max_scarce_iter_count) {
        return {uv, st};
      }
    } else {
      scarce_iter_count = 0;
    }

    if (dist_new < s_conj_grad_accuracy / 4) {
      return {uv_new, st_new};
    }
    if (dist_new < dist) {
      step = s_conj_grad_step;
      dist = dist_new;
      uv = uv_new;
      st = st_new;
    } else {
      step *= 0.5f;
    }
  }

  return {uv, st};
}

glm::vec3 IntersectionBuilder::get_surface_position(mge::Entity& s, const glm::vec2& uv) {
  if (s.has_component<TorusComponent>()) {
    return s.get_component<mge::TransformComponent>().get_model_mat() * glm::vec4(s.get_component<TorusComponent>().get_uv_pos(uv), 1.0);
  }
  else if (s.has_component<BezierSurfaceC0Component>()) {
    return s.get_component<BezierSurfaceC0Component>().get_uv_pos(uv);
  }
  else if (s.has_component<BezierSurfaceC2Component>()) {
    return s.get_component<BezierSurfaceC2Component>().get_uv_pos(uv);
  }

  return {};
}

std::pair<glm::vec3, glm::vec3> IntersectionBuilder::get_surface_gradient(mge::Entity& s, const glm::vec2& uv) {
  if (s.has_component<TorusComponent>()) {
    auto model = s.get_component<mge::TransformComponent>().get_model_mat();
    auto grad = s.get_component<TorusComponent>().get_uv_grad(uv);
    return {model * glm::vec4(grad.first, 0.0f), model * glm::vec4(grad.second, 0.0f)};
  }
  else if (s.has_component<BezierSurfaceC0Component>()) {
    return s.get_component<BezierSurfaceC0Component>().get_uv_grad(uv);
  }
  else if (s.has_component<BezierSurfaceC2Component>()) {
    return s.get_component<BezierSurfaceC2Component>().get_uv_grad(uv);
  }

  return {};
}

float IntersectionBuilder::penalty_function(glm::vec2 uv, glm::vec2 st) {
  return std::fmax(0.0f, (1 - glm::distance(uv, st) / s_penalty_length) * s_max_penalty);
}

glm::vec2 IntersectionBuilder::normalize_parametrization(mge::Entity& s, glm::vec2 uv) {
  bool wrapped_u = false, wrapped_v = false;
  if (s.has_component<TorusComponent>()) {
    wrapped_u = true;
    wrapped_v = true;
  }
  else if (s.has_component<BezierSurfaceC0Component>()) {
    wrapped_u = s.get_component<BezierSurfaceC0Component>().get_wrapping() == BezierSurfaceWrapping::u;
    wrapped_v = s.get_component<BezierSurfaceC0Component>().get_wrapping() == BezierSurfaceWrapping::v;
  }
  else if (s.has_component<BezierSurfaceC2Component>()) {
    wrapped_u = s.get_component<BezierSurfaceC2Component>().get_wrapping() == BezierSurfaceWrapping::u;
    wrapped_v = s.get_component<BezierSurfaceC2Component>().get_wrapping() == BezierSurfaceWrapping::v;
  }
  return {normalize_parameter(uv.x, wrapped_u), normalize_parameter(uv.y, wrapped_v)};
}

bool IntersectionBuilder::are_parameters_normalized(mge::Entity& s, glm::vec2 uv) {
  auto wrapped_u = false, wrapped_v = false;
  if (s.has_component<TorusComponent>()) {
    wrapped_u = true;
    wrapped_v = true;
  }
  else if (s.has_component<BezierSurfaceC0Component>()) {
    wrapped_u = s.get_component<BezierSurfaceC0Component>().get_wrapping() == BezierSurfaceWrapping::u;
    wrapped_v = s.get_component<BezierSurfaceC0Component>().get_wrapping() == BezierSurfaceWrapping::v;
  }
  else if (s.has_component<BezierSurfaceC2Component>()) {
    wrapped_u = s.get_component<BezierSurfaceC2Component>().get_wrapping() == BezierSurfaceWrapping::u;
    wrapped_v = s.get_component<BezierSurfaceC2Component>().get_wrapping() == BezierSurfaceWrapping::v;
  }
  return (wrapped_u || (uv.x >= 0.0 && uv.x <= 1.0)) && (wrapped_v || (uv.y >= 0.0 && uv.y <= 1.0));
}

glm::vec2 IntersectionBuilder::clamp_parameters(mge::Entity& s, glm::vec2 uv) {
  auto wrapped_u = false, wrapped_v = false;
  if (s.has_component<TorusComponent>()) {
    wrapped_u = true;
    wrapped_v = true;
  }
  else if (s.has_component<BezierSurfaceC0Component>()) {
    wrapped_u = s.get_component<BezierSurfaceC0Component>().get_wrapping() == BezierSurfaceWrapping::u;
    wrapped_v = s.get_component<BezierSurfaceC0Component>().get_wrapping() == BezierSurfaceWrapping::v;
  }
  else if (s.has_component<BezierSurfaceC2Component>()) {
    wrapped_u = s.get_component<BezierSurfaceC2Component>().get_wrapping() == BezierSurfaceWrapping::u;
    wrapped_v = s.get_component<BezierSurfaceC2Component>().get_wrapping() == BezierSurfaceWrapping::v;
  }
  return {
    (wrapped_u) ? uv.x : glm::clamp(uv.x, 0.0f, 1.0f),
    (wrapped_v) ? uv.y : glm::clamp(uv.y, 0.0f, 1.0f)
  };
}

std::pair<glm::vec2, glm::vec2> IntersectionBuilder::get_gradients(mge::Entity& s1, mge::Entity& s2, glm::vec2 uv, glm::vec2 st) {
  auto pos1 = get_surface_position(s1, uv), pos2 = get_surface_position(s2, st), delta = pos1 - pos2;
  auto grad1 = get_surface_gradient(s1, uv), grad2 = get_surface_gradient(s2, st);

  return {
    glm::vec2(glm::dot(delta, grad1.first), glm::dot(delta, grad1.second)),
    glm::vec2(glm::dot(-delta, grad2.first), glm::dot(-delta, grad2.second))
  };
}