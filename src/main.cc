#include "mcad_application.hh"

int main() {
  auto app = MCadApplication();
  app.run();
  app.terminate();
  return 0;
}