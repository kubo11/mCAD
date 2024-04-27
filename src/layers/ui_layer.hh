#ifndef MCAD_UI_LAYER_HH
#define MCAD_UI_LAYER_HH

#include "mge.hh"

#include "../events/events.hh"
#include "../geometry/point_component.hh"
#include "../geometry/selectible_component.hh"
#include "../geometry/torus_component.hh"

enum class UIMode {
  NONE,
  ADD_BEZIER_POINT,
  REMOVE_BEZIER_POINT,
};

class UILayer : public mge::Layer {
 public:
  UILayer();
  ~UILayer() { MGE_WARN("UILayer terminated."); }

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  std::optional<std::reference_wrapper<mge::Entity>> m_displayed_entity;
  std::map<std::string, bool> m_entities;
  UIMode m_ui_mode;
  InputState m_input_state;

  void show_tag_panel(const mge::Entity& entity);
  void show_transform_panel(const mge::Entity& entity);
  void show_limited_transform_panel(const mge::Entity& entity);
  void show_renderable_component(const mge::Entity& entity);
  void show_torus_panel(const mge::Entity& entity);
  void show_bezier_panel(const mge::Entity& entity);
  void show_tools_panel();
  void show_entities_list_panel();
  void show_entity_parameters_panel(const mge::Entity& entity);

  bool on_new_entity(NewEntityEvent& event);
  bool on_removed_entity(RemoveEntityEvent& event);
  bool on_select_entity_by_tag(SelectEntityByTagEvent& event);
  bool on_select_entity_by_position(SelectEntityByPositionEvent& event);
  bool on_unselect_all_entities(UnSelectAllEntitiesEvent event);
  bool on_unselect_entity_by_tag(UnSelectEntityByTagEvent event);
  bool on_update_displayed_entity(UpdateDisplayedEntityEvent& event);
};

#endif  // MCAD_UI_LAYER_HH