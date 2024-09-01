#ifndef MCAD_GRID_LAYER
#define MCAD_GRID_LAYER

#include "mge.hh"

#include "../events/events.hh"
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
  bool m_do_anaglyphs;

  // Anaglyph events
  bool on_anaglyph_update_state(AnaglyphUpdateStateEvent& event);
};

#endif  // MCAD_GRID_LAYER