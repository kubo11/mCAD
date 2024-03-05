#include "mcad_application.hh"
#include "layers/ellipsoid_layer.hh"
#include "layers/ui_layer.hh"

MCadApplication::MCadApplication() {
  push_layer(std::move(std::make_unique<EllipsoidLayer>()));
  push_layer(std::move(std::make_unique<UILayer>()));
}