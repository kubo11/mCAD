#ifndef MCAD_GRID_LAYER
#define MCAD_GRID_LAYER

#include "mge.hh"

#include "../vertices/grid_vertex.hh"

class GridLayer : public mge::Layer {
 public:
  GridLayer(mge::Scene& scene);
  ~GridLayer() {}

  virtual void configure() override;
  virtual void update() override;

 private:
  std::unique_ptr<mge::RenderPipeline<GridVertex>> m_render_pipeline = nullptr;
  std::unique_ptr<mge::RenderableComponent<GridVertex>> m_grid = nullptr;
  mge::Scene& m_scene;
};

#endif  // MCAD_GRID_LAYER