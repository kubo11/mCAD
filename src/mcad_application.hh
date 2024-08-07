#ifndef MCAD_APPLICATION_HH
#define MCAD_APPLICATION_HH

#include "mge.hh"

#include "events/events.hh"

class MCadApplication : public mge::Application {
 public:
  MCadApplication();

 protected:
  std::shared_ptr<EventManager> m_event_manager = nullptr;
};

#endif  // MCAD_APPLICATION_HH