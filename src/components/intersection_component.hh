#ifndef MCAD_INTERSECTION_COMPONENT
#define MCAD_INTERSECTION_COMPONENT

#include "mge.hh"

struct IntersectionComponent {
 public:
  IntersectionComponent(mge::Entity& intersectable1, mge::Entity& intersectable2, const std::vector<glm::vec2>& points1, const std::vector<glm::vec2>& points2);

  static std::string get_new_name() { return "Intersection " + std::to_string(s_new_id++); }
  const std::vector<glm::vec3> get_points() const { return m_points; }

  const std::pair<int, int> get_texture_ids() const;

 private:
  static unsigned int s_new_id;
  mge::Entity& m_intersectable1;
  mge::Entity& m_intersectable2;
  std::vector<glm::vec3> m_points;
  mge::Texture m_texture1;
  mge::Texture m_texture2;
  mge::Canvas m_canvas1;
  mge::Canvas m_canvas2;

  static void tex_apply(mge::Canvas& c, mge::Texture& t, const std::vector<glm::vec2> &uv);
  static void tex_apply_same(mge::Canvas& c, mge::Texture& t, const std::vector<glm::vec2> &uv, const std::vector<glm::vec2> &st);
  static std::pair<glm::vec2, glm::vec2> get_most_probable_loop(const std::vector<glm::vec2> &s_points, bool wrap_u, bool wrap_v);
};

#endif // MCAD_INTERSECTION_COMPONENT