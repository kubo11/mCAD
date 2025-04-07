#ifndef MCAD_EVENTS_GREGORY_PATCH_EVENT
#define MCAD_EVENTS_GREGORY_PATCH_EVENT

#include "mge.hh"

enum class GregoryPatchEvents { FindHoles, Add, UpdateLineCount, UpdateVectorsState };

class FindHoleEvent : public mge::Event<GregoryPatchEvents> {
 public:
  FindHoleEvent(const std::vector<mge::EntityId>& patch_ids)
    : mge::Event<GregoryPatchEvents>(GregoryPatchEvents::FindHoles, "FindHoleEvent"), patch_ids(patch_ids) {}

  std::vector<mge::EntityId> patch_ids;
  std::vector<std::vector<mge::EntityId>> hole_ids;
};

class AddGregoryPatchEvent : public mge::Event<GregoryPatchEvents> {
 public:
  AddGregoryPatchEvent(const std::vector<mge::EntityId>& hole_ids)
    : mge::Event<GregoryPatchEvents>(GregoryPatchEvents::Add, "AddGregoryPatch"), hole_ids(hole_ids){}

  std::vector<mge::EntityId> hole_ids;
};

class GregoryPatchUpdateVectorsStateEvent : public mge::Event<GregoryPatchEvents> {
  public:
   GregoryPatchUpdateVectorsStateEvent(mge::EntityId id, bool state)
       : mge::Event<GregoryPatchEvents>(GregoryPatchEvents::UpdateVectorsState,
                                           "GregoryPatchUpdateVectorsStateEvent"),
         id(id),
         state(state) {}
 
   mge::EntityId id;
   bool state;
 };
 
 class GregoryPatchUpdateLineCountEvent : public mge::Event<GregoryPatchEvents> {
  public:
   GregoryPatchUpdateLineCountEvent(mge::EntityId id, unsigned int line_count)
       : mge::Event<GregoryPatchEvents>(GregoryPatchEvents::UpdateLineCount,
                                           "GregoryPatchUpdateLineCountEvent"),
         id(id),
         line_count(line_count) {}
 
   mge::EntityId id;
   unsigned int line_count;
 };

#endif  // MCAD_EVENTS_GREGORY_PATCH_EVENT