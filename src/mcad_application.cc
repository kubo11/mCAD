#include "mcad_application.hh"
#include "layers/cad_layer.hh"
#include "layers/grid_layer.hh"
#include "layers/ui_layer.hh"

MCadApplication::MCadApplication() {
  m_event_manager = EventManager::create(*m_timer);
  m_scene->get_current_camera().set_velocity(10.0f);
  auto cad_layer =
      std::make_unique<CadLayer>(*m_scene, glm::ivec2{m_main_window->get_width(), m_main_window->get_height()});
  auto& mass_center = cad_layer->get_mass_center();
  push_layer(std::move(cad_layer));
  push_layer(std::move(std::make_unique<GridLayer>(*m_scene)));
  push_layer(std::move(std::make_unique<UILayer>(mass_center)));
}