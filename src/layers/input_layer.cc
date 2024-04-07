#include "input_layer.hh"

#include "../events/events.hh"

InputLayer::InputLayer() {}

void InputLayer::configure() {}

void InputLayer::update() {}

void InputLayer::handle_event(mge::Event& event, float dt) {
  mge::Event::try_handler<InputStateChangedEvent>(
      event, BIND_EVENT_HANDLER(InputLayer::on_input_state_changed));
  mge::Event::try_handler<mge::WindowMouseMovedEvent>(
      event, BIND_EVENT_HANDLER(InputLayer::on_mouse_moved));
  mge::Event::try_handler<mge::WindowScrollEvent>(
      event, BIND_EVENT_HANDLER(InputLayer::on_mouse_scroll));
  mge::Event::try_handler<mge::WindowMousePressedEvent>(
      event, BIND_EVENT_HANDLER(InputLayer::on_mouse_button_pressed));
}

bool InputLayer::on_input_state_changed(InputStateChangedEvent& event) {
  m_state = event.get_input_state();
  return false;
}

bool InputLayer::on_mouse_moved(mge::WindowMouseMovedEvent& event) {
  const mge::Window& window = event.get_window();
  if (window.is_key_pressed(GLFW_KEY_LEFT_SHIFT)) {
    send_camera_move_events(event);
    return true;
  }

  if (window.is_key_pressed(GLFW_KEY_LEFT_CONTROL)) {
    CursorMoveEvent cursor_event(event.get_end());
    m_send_event(cursor_event);
    return true;
  }

  switch (m_state) {
    case InputState::SCALE:
      if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        ScaleByCursorEvent scale_event(event.get_beg(), event.get_end());
        m_send_event(scale_event);
      }
      break;
    case InputState::ROTATEX:
      if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        RotateByCursorEvent rotate_event(event.get_beg(), event.get_end(),
                                         {1.0f, 0.0f, 0.0f});
        m_send_event(rotate_event);
      }
      break;
    case InputState::ROTATEY:
      if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        RotateByCursorEvent rotate_event(event.get_beg(), event.get_end(),
                                         {0.0f, 1.0f, 0.0f});
        m_send_event(rotate_event);
      }
      break;
    case InputState::ROTATEZ:
      if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        RotateByCursorEvent rotate_event(event.get_beg(), event.get_end(),
                                         {0.0f, 0.0f, 1.0f});
        m_send_event(rotate_event);
      }
      break;
    case InputState::MOVE:
      if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        MoveByCursorEvent move_event;
        m_send_event(move_event);
      }
      break;
    default:
      break;
  }
  return true;
}

bool InputLayer::on_mouse_button_pressed(mge::WindowMousePressedEvent& event) {
  const mge::Window& window = event.get_window();
  if (window.is_key_pressed(GLFW_KEY_LEFT_SHIFT)) {
    return true;
  }

  switch (m_state) {
    case InputState::SELECT:
      if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        SelectEntityByPositionEvent select_event(event.get_position());
        m_send_event(select_event);
      }
      break;
    case InputState::DELETE:
      if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        DeletePositionEvent delete_event(event.get_position());
        m_send_event(delete_event);
      }
      break;
    default:
      break;
  }
  return true;
}

bool InputLayer::on_mouse_scroll(mge::WindowScrollEvent& event) {
  send_camera_zoom_event(event);

  return true;
}

void InputLayer::send_camera_move_events(mge::WindowMouseMovedEvent& event) {
  const mge::Window& window = event.get_window();
  if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
    float sensitivity = 1.0f;
    mge::CameraAngleEvent cam_event(sensitivity * event.get_offset().x,
                                    sensitivity * event.get_offset().y);
    window.send_event(cam_event);
    return;
  }

  if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    float sensitivity = 0.5f;
    mge::CameraPositionEvent cam_event(
        glm::vec2(-sensitivity * event.get_offset().x,
                  -sensitivity * event.get_offset().y));
    window.send_event(cam_event);
    return;
  }

  if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
    float sensitivity = 1.005f;
    mge::CameraZoomEvent cam_event(std::pow(sensitivity, event.get_offset().x));
    window.send_event(cam_event);
    return;
  }
}

void InputLayer::send_camera_zoom_event(mge::WindowScrollEvent& event) {
  const mge::Window& window = event.get_window();
  float sensitivity = 1.1f;
  mge::CameraZoomEvent cam_event(std::pow(sensitivity, -event.get_y_offset()));
  window.send_event(cam_event);
}