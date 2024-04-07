#ifndef MCAD_UI_LAYER_HH
#define MCAD_UI_LAYER_HH

#include "mge.hh"

#include "../events/events.hh"
#include "../geometry/point_component.hh"
#include "../geometry/selectible_component.hh"
#include "../geometry/torus_component.hh"

class UILayer : public mge::Layer {
  using SelectibleEntities =
      entt::get_t<SelectibleComponent, mge::TagComponent>;
  using SelectedEntities = entt::get_t<SelectedComponent, mge::TagComponent>;
  using SelectedChildEntities =
      entt::get_t<SelectedChildComponent, mge::TagComponent>;
  using NoExclude = entt::exclude_t<>;

 public:
  UILayer(const mge::Scene& scene);
  ~UILayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  const mge::Scene& m_scene;

  void show_tag_panel(const mge::TagComponent& component);
  void show_transform_panel(const mge::TransformComponent& component);
  void show_limited_transform_panel(const mge::TransformComponent& component);
  void show_renderable_component(
      const mge::RenderableComponent<GeometryVertex>& component);
  void show_torus_panel(const TorusComponent& component);
  void show_point_panel(const PointComponent& component);
  void show_tools_panel();
  void show_entities_list_panel();
  void show_entity_parameters_panel(const mge::Entity& entity);
};

#endif  // MCAD_UI_LAYER_HH