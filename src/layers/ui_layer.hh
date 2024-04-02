#ifndef MCAD_UI_LAYER_HH
#define MCAD_UI_LAYER_HH

#include "mge.hh"

#include "../events/events.hh"
#include "../geometry/point_component.hh"
#include "../geometry/torus_component.hh"

class UILayer : public mge::Layer {
 public:
  UILayer(mge::Entity& cursor);
  ~UILayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  std::map<std::string, bool> m_entities;

  bool on_announce_new_entity(AnnounceNewEntityEvent& event);
  bool on_select_entity_by_tag(SelectEntityByTagEvent& event);
  bool on_delete_entity_by_tag(DeleteEntityByTagEvent& event);
  bool on_unselect_entity_by_tag(UnSelectEntityByTagEvent& event);
  bool on_unselect_all_entities(UnSelectAllEntitiesEvent& event);

  void show_tag_panel(mge::TagComponent& component);
  void show_transform_panel(mge::TransformComponent& component);
  void show_limited_transform_panel(mge::TransformComponent& component);
  bool show_renderable_component(
      mge::RenderableComponent<GeometryVertex>& component);
  std::pair<bool, bool> show_torus_panel(TorusComponent& component);
  void show_point_panel(PointComponent& component);
  void show_tools_panel();
  void show_entities_list_panel();
  void show_entity_parameters_panel(mge::Entity& entity);
};

#endif  // MCAD_UI_LAYER_HH