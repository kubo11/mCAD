#ifndef MCAD_EVENTS_CURSOR_EVENT
#define MCAD_EVENTS_CURSOR_EVENT

#include "mge.hh"

enum class CursorEvents { Move };

class CursorMoveEvent : public mge::Event<CursorEvents> {
 public:
  CursorMoveEvent(glm::vec2 screen_space_position)
      : mge::Event<CursorEvents>(CursorEvents::Move, "CursorMoveEvent"), screen_space_position(screen_space_position) {}

  glm::vec2 screen_space_position;
};

#endif  // MCAD_EVENTS_CURSOR_EVENT