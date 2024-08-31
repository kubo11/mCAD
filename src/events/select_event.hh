#ifndef MCAD_EVENTS_SELECT_EVENT
#define MCAD_EVENTS_SELECT_EVENT

#include "mge.hh"

enum class SelectionEvents { SelectionUpdate, UnselectAllEntities, Degrade };

class SelectionUpdateEvent : public mge::Event<SelectionEvents> {
 public:
  SelectionUpdateEvent(mge::EntityId id, bool selection, bool is_parent)
      : mge::Event<SelectionEvents>(SelectionEvents::SelectionUpdate, "SelectionUpdateEvent"),
        id(id),
        selection(selection),
        is_parent(is_parent) {}

  mge::EntityId id;
  bool selection;
  bool is_parent;
};

class UnselectAllEntitiesEvent : public mge::Event<SelectionEvents> {
 public:
  UnselectAllEntitiesEvent()
      : mge::Event<SelectionEvents>(SelectionEvents::UnselectAllEntities, "UnselectAllEntitiesEvent") {}
};

class DegradeSelectionEvent : public mge::Event<SelectionEvents> {
 public:
  DegradeSelectionEvent(mge::EntityId id)
      : mge::Event<SelectionEvents>(SelectionEvents::Degrade, "DegradeSelectionEvent"), id(id) {}

  mge::EntityId id;
};

#endif  // MCAD_EVENTS_SELECT_EVENT