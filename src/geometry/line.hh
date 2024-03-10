#ifndef MCAD_GEOMETRY_LINE
#define MCAD_GEOMETRY_LINE

#include "mge.hh"

#include "point.hh"

class Line : public mge::Drawable {
 public:
  Line(Point& beg, Point& end)
      : m_beg(beg),
        m_end(end),
        m_vertex_array(std::make_unique<mge::VertexArray<GeometryVertex>>(
            std::vector<GeometryVertex>{GeometryVertex(beg.get_position()),
                                        GeometryVertex(end.get_position())},
            GeometryVertex::get_vertex_attributes(),
            std::vector<unsigned int>{1u, 2u})),
        m_shader(mge::ShaderSystem::acquire(fs::current_path() / "src" /
                                            "shaders" / "solid" / "surface")) {}
  ~Line() = default;

  virtual void init_draw_data() override;
  virtual void draw() const override;

 private:
  Point& m_beg;
  Point& m_end;
  glm::vec3 m_color = {0.0f, 0.0f, 0.0f};
  mge::Shader& m_shader;
  std::unique_ptr<mge::VertexArray<GeometryVertex>> m_vertex_array;
};

#endif  // MCAD_GEOMETRY_LINE