#ifndef MCAD_LAYERS_INPUT_LAYER_HH
#define MCAD_LAYERS_INPUT_LAYER_HH

#include "mge.hh"

#include "../events/input_state_event.hh"
#include "../input_state.hh"

class InputLayer : public mge::Layer {
 public:
  InputLayer();
  ~InputLayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  InputState m_state = InputState::SELECT;

  void send_camera_move_events(mge::WindowMouseMovedEvent& event);
  void send_camera_zoom_event(mge::WindowScrollEvent& event);

  bool on_input_state_changed(InputStateChangedEvent& event);
  bool on_mouse_moved(mge::WindowMouseMovedEvent& event);
  bool on_mouse_button_pressed(mge::WindowMousePressedEvent& event);
  bool on_mouse_scroll(mge::WindowScrollEvent& event);
};

#endif  // MCAD_LAYERS_INPUT_LAYER_HH