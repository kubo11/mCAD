#ifndef MCAD_INTERSECTION_COMPONENT
#define MCAD_INTERSECTION_COMPONENT

#include "mge.hh"

struct IntersectionComponent {
 public:
  IntersectionComponent(mge::Entity& intersectable1, mge::Entity& intersectable2, const std::vector<glm::vec2>& points1, const std::vector<glm::vec2>& points2);

  static std::string get_new_name() { return "Intersection " + std::to_string(s_new_id++); }
  const std::vector<glm::vec3> get_points() const { return m_points; }

  const std::pair<int, int> get_texture_ids() const;

  void use_texture_for(const mge::Entity& intersectable, int slot);

  void update_trim(glm::vec2 uv, bool first);

  bool get_hide_points_status() const;
  void set_hide_points_status(bool status);

 private:
  static constexpr int s_texture_size = 256;
  static unsigned int s_new_id;
  mge::Entity& m_intersectable1;
  mge::Entity& m_intersectable2;
  std::vector<glm::vec3> m_points;
  mge::Texture m_texture1;
  mge::Texture m_texture2;
  mge::Canvas m_canvas1;
  mge::Canvas m_canvas2;
  bool m_hide_points_status = false;

  static void tex_apply(mge::Canvas& c, mge::Texture& t, const std::vector<glm::vec2> &uv);
  static void tex_apply_same(mge::Canvas& c, mge::Texture& t, const std::vector<glm::vec2> &uv, const std::vector<glm::vec2> &st);
  static std::pair<glm::vec2, glm::vec2> find_loop_gap(const std::vector<glm::vec2> &points, bool wrap_u, bool wrap_v);
  void update_trim(glm::vec2 uv, mge::Canvas& canvas, mge::Texture& texture);
  void update_points_status(mge::Entity& intersectable, mge::Canvas& canvas);
};

#endif // MCAD_INTERSECTION_COMPONENT