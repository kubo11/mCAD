#ifndef MCAD_GEOMETRY_CURSOR_COMPONENT_HH
#define MCAD_GEOMETRY_CURSOR_COMPONENT_HH

#include "mge.hh"

struct CursorComponent {
  CursorComponent() {}
  CursorComponent(CursorComponent&&) = default;
  inline CursorComponent& operator=(CursorComponent&& other) { return *this; }

  void on_construct(mge::Entity& entity);

 private:
  int _ = 1;
};

#endif  // MCAD_GEOMETRY_CURSOR_COMPONENT_HH