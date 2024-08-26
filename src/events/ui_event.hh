#ifndef MCAD_EVENTS_UI_EVENT
#define MCAD_EVENTS_UI_EVENT

#include "mge.hh"

enum class UIEvents { SelectionUpdate };

class UISelectionUpdateEvent : public mge::Event<UIEvents> {
 public:
  UISelectionUpdateEvent(mge::EntityId id, bool state, bool is_parent)
      : mge::Event<UIEvents>(UIEvents::SelectionUpdate, "UISelectionUpdateEvent"),
        id(id),
        state(state),
        is_parent(is_parent) {}

  mge::EntityId id;
  bool state;
  bool is_parent;
};

#endif  // MCAD_EVENTS_UI_EVENT