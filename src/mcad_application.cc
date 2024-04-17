#include "mcad_application.hh"
#include "layers/cad_layer.hh"
#include "layers/grid_layer.hh"
#include "layers/input_layer.hh"
#include "layers/ui_layer.hh"

MCadApplication::MCadApplication() {
  auto cad_layer = std::make_unique<CadLayer>(m_scene);
  push_layer(std::move(cad_layer));
  //   push_layer(std::move(std::make_unique<GridLayer>(m_camera)));
  push_layer(std::move(std::make_unique<UILayer>()));
  push_layer(std::move(std::make_unique<InputLayer>()));
}