#ifndef MCAD_GRID_LAYER_HH
#define MCAD_GRID_LAYER_HH

#include "mge.hh"

#include "../geometry/geometry_vertex.hh"

class GridLayer : public mge::Layer {
 public:
  GridLayer(mge::Camera& camera);
  ~GridLayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  std::unique_ptr<mge::VertexArray<GeometryVertex>> m_vertex_array = nullptr;
  mge::Shader& m_shader;
  mge::Camera& m_camera;
};

#endif  // MCAD_GRID_LAYER_HH