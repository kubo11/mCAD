#include "intersection_component.hh"

#include "components/bezier_surface_c0_component.hh"
#include "components/bezier_surface_c2_component.hh"
#include "components/torus_component.hh"

unsigned int IntersectionComponent::s_new_id = 1;

glm::vec3 position(mge::Entity& e, glm::vec2 uv) {
  if (e.has_component<TorusComponent>()) {
    return e.get_component<mge::TransformComponent>().get_model_mat() * glm::vec4(e.get_component<TorusComponent>().get_uv_pos(uv), 1.0f);
  }
  else if (e.has_component<BezierSurfaceC0Component>()) {
    return e.get_component<BezierSurfaceC0Component>().get_uv_pos(uv);
  }
  else if (e.has_component<BezierSurfaceC2Component>()) {
    return e.get_component<BezierSurfaceC2Component>().get_uv_pos(uv);
  }
  return glm::vec3(0.0f);
}

bool wrap_x(mge::Entity& e) {
  if (e.has_component<TorusComponent>()) {
    return true;
  }
  else if (e.has_component<BezierSurfaceC0Component>()) {
    return e.get_component<BezierSurfaceC0Component>().get_wrapping() == BezierSurfaceWrapping::u;
  }
  else if (e.has_component<BezierSurfaceC2Component>()) {
    return e.get_component<BezierSurfaceC2Component>().get_wrapping() == BezierSurfaceWrapping::u;
  }
  return false;
}

bool wrap_y(mge::Entity& e) {
  if (e.has_component<TorusComponent>()) {
    return true;
  }
  else if (e.has_component<BezierSurfaceC0Component>()) {
    return e.get_component<BezierSurfaceC0Component>().get_wrapping() == BezierSurfaceWrapping::v;
  }
  else if (e.has_component<BezierSurfaceC2Component>()) {
    return e.get_component<BezierSurfaceC2Component>().get_wrapping() == BezierSurfaceWrapping::v;
  }
  return false;
}

IntersectionComponent::IntersectionComponent(mge::Entity& intersectable1, mge::Entity& intersectable2, const std::vector<glm::vec2>& points1, const std::vector<glm::vec2>& points2)
 : m_intersectable1{intersectable1}, m_intersectable2{intersectable2}, m_points{}, m_texture1{{s_texture_size, s_texture_size}}, m_texture2{{s_texture_size, s_texture_size}}, 
    m_canvas1{{s_texture_size, s_texture_size}, mge::Color::White, wrap_x(intersectable1), wrap_y(intersectable1)},\
    m_canvas2{{s_texture_size, s_texture_size}, mge::Color::White, wrap_x(intersectable2), wrap_y(intersectable2)} {
  m_points.reserve(points1.size());
  for (const auto& point : points1) {
    m_points.push_back(position(intersectable1, point));
  }
  if (intersectable1 == intersectable2) {
    tex_apply_same(m_canvas1, m_texture1, points1, points2);
  }
  else {
      tex_apply(m_canvas1, m_texture1, points1);
      tex_apply(m_canvas2, m_texture2, points2);
  }
 }

const std::pair<int, int> IntersectionComponent::get_texture_ids() const {
  return {m_texture1.get_id(), m_intersectable1 != m_intersectable2 ? m_texture2.get_id() : -1};
}

void IntersectionComponent::tex_apply(mge::Canvas& c, mge::Texture& t, const std::vector<glm::vec2> &uv) {
  c.clear();
  for (int i = 1; i < uv.size(); i++) {
    c.line(uv[i - 1], uv[i]);
  }
  auto l = get_most_probable_loop(uv, c.get_wrap_x(), c.get_wrap_y());
  c.line(l.first, l.second);
  t.use();
  t.copy(c.get_data());
}

void IntersectionComponent::tex_apply_same(mge::Canvas& c, mge::Texture& t, const std::vector<glm::vec2> &uv, const std::vector<glm::vec2> &st) {
  c.clear();
  for (int i = 1; i < uv.size(); i++) {
    c.line(uv[i - 1], uv[i]);
  }
  auto l1 = get_most_probable_loop(uv, c.get_wrap_x(), c.get_wrap_y());
  c.line(l1.first, l1.second);
  for (int i = 1; i < st.size(); i++) {
    c.line(st[i - 1], st[i]);
  }
  auto l2 = get_most_probable_loop(st, c.get_wrap_x(), c.get_wrap_y());
  c.line(l2.first, l2.second);
  t.use();
  t.copy(c.get_data());
}

std::pair<glm::vec2, glm::vec2> IntersectionComponent::get_most_probable_loop(const std::vector<glm::vec2> &s_points, bool wrap_u, bool wrap_v) {
  glm::vec2 first = s_points.front(), last = s_points.back();
  first.x = std::fmod(first.x, 1.0f);
  first.x += first.x < 0.0f ? 1.0f : 0.0f;
  first.y = std::fmod(first.y, 1.0f);
  first.y += first.y < 0.0f ? 1.0f : 0.0f;
  last.x = std::fmod(last.x, 1.0f);
  last.x += last.x < 0.0f ? 1.0f : 0.0f;
  last.y = std::fmod(last.y, 1.0f);
  last.y += last.y < 0.0f ? 1.0f : 0.0f;
  std::tuple best(first, last, glm::distance(first, last)); // distance in UV space
  std::vector<decltype(best)> candidates;
  if (wrap_u) {
    candidates.emplace_back(first, last + glm::vec2(1.0, 0.0), glm::distance(first, last + glm::vec2(1.0, 0.0)));
    candidates.emplace_back(first, last - glm::vec2(1.0, 0.0), glm::distance(first, last - glm::vec2(1.0, 0.0)));
  }
  if (wrap_v) {
    candidates.emplace_back(first, last + glm::vec2(0.0, 1.0), glm::distance(first, last + glm::vec2(0.0, 1.0)));
    candidates.emplace_back(first, last - glm::vec2(0.0, 1.0), glm::distance(first, last - glm::vec2(0.0, 1.0)));
  }
  for(auto [f, l, d] : candidates) {
    float best_dist = std::get<2>(best);
    if (d < best_dist) {
      best = {f,l,d};
    }
  }
  return { std::get<0>(best), std::get<1>(best) };
}

void IntersectionComponent::use_texture_for(const mge::Entity& intersectable, int slot) {
  if (m_intersectable1 == intersectable) {
    m_texture1.use(slot);
  }
  else if (m_intersectable2 == intersectable) {
    m_texture2.use(slot);
  }
}

void IntersectionComponent::update_trim(glm::vec2 uv, bool first) {
  if (first) {
    update_trim(uv, m_canvas1, m_texture1);
    if (m_hide_points_status) {
      if (m_intersectable1.has_component<BezierSurfaceC0Component>()) {
        m_intersectable1.patch<BezierSurfaceC0Component>([&canvas=m_canvas1](auto& bezier) {bezier.update_points_status(canvas);});
      }
      else if (m_intersectable1.has_component<BezierSurfaceC2Component>()) {
        m_intersectable1.patch<BezierSurfaceC2Component>([&canvas=m_canvas1](auto& bezier) {bezier.update_points_status(canvas);});
      }
    }
  }
  else {
    update_trim(uv, m_canvas2, m_texture2);
    if (m_hide_points_status) {
      if (m_intersectable2.has_component<BezierSurfaceC0Component>()) {
        m_intersectable2.patch<BezierSurfaceC0Component>([&canvas=m_canvas2](auto& bezier) {bezier.update_points_status(canvas);});
      }
      else if (m_intersectable2.has_component<BezierSurfaceC2Component>()) {
        m_intersectable2.patch<BezierSurfaceC2Component>([&canvas=m_canvas2](auto& bezier) {bezier.update_points_status(canvas);});
      }
    }
  }
}

void IntersectionComponent::update_trim(glm::vec2 uv, mge::Canvas& canvas, mge::Texture& texture) {
  if (canvas.get_pixel(uv) == mge::Color::Black) return;

  if (canvas.get_pixel(uv) == mge::Color::White) {
    canvas.set_color(mge::Color::Red);
  } else {
    canvas.set_color(mge::Color::White);
  }

  canvas.flood_fill(uv);
  texture.use();
  texture.copy(canvas.get_data());
}

bool IntersectionComponent::get_hide_points_status() const {
  return m_hide_points_status;
}

void IntersectionComponent::set_hide_points_status(bool status) {
  if (m_hide_points_status == status) return;
  m_hide_points_status = status;
  if (!m_intersectable1.has_component<TorusComponent>()) {
    update_points_status(m_intersectable1, m_canvas1);
  }
  if (m_intersectable1 != m_intersectable2 && !m_intersectable2.has_component<TorusComponent>()) {
    update_points_status(m_intersectable2, m_canvas2);
  }
}

void IntersectionComponent::update_points_status(mge::Entity& intersectable, mge::Canvas& canvas) {
  if (intersectable.has_component<BezierSurfaceC0Component>()) {
    if (m_hide_points_status) {
      intersectable.patch<BezierSurfaceC0Component>([&canvas](auto& surface){
        surface.update_points_status(canvas);
      });
    }
    else {
      intersectable.patch<BezierSurfaceC0Component>([](auto& surface){
        surface.show_all_points();
      });
    }
  }
  else if (intersectable.has_component<BezierSurfaceC2Component>()) {
    if (m_hide_points_status) {
      intersectable.patch<BezierSurfaceC2Component>([&canvas](auto& surface){
        surface.update_points_status(canvas);
      });
    }
    else {
      intersectable.patch<BezierSurfaceC2Component>([](auto& surface){
        surface.show_all_points();
      });
    }
  }
}