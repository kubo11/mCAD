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
  std::unordered_map<std::string, bool> m_entities;
  std::vector<std::reference_wrapper<mge::Entity>> m_selected;
  mge::Entity& m_cursor;

  void select(mge::Entity& entity);
  void unselect(mge::Entity& entity);
  void unselect_all();

  bool on_selected(SelectEvent& event);
  bool on_announce_new(AnnounceNewEvent& event);
  bool on_delete_tag(DeleteTagEvent& event);
  bool on_move_event(MoveEvent& event);
  bool on_rotate_event(RotateEvent& event);
  bool on_scale_event(ScaleEvent& event);
  bool on_draw_mass_center_event(DrawMassCenterEvent& event);

  void show_tag_panel(mge::TagComponent& component);
  void show_transform_panel(mge::TransformComponent& component);
  bool show_renderable_component(
      mge::RenderableComponent<GeometryVertex>& component);
  std::pair<bool, bool> show_torus_panel(TorusComponent& component);
  void show_point_panel(PointComponent& component);
  void show_tools_panel();
  void show_entities_list_panel();
  void show_entity_parameters_panel(mge::Entity& entity);
};

#endif  // MCAD_UI_LAYER_HH