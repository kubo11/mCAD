#ifndef MCAD_EVENTS_TORUS_EVENT_HH
#define MCAD_EVENTS_TORUS_EVENT_HH

#include "mge.hh"

class TorusUpdateEvent : public mge::Event {
 public:
  TorusUpdateEvent(const std::string& tag) : m_tag(tag) {}
  virtual inline const std::string name() const override {
    return "TorusUpdateEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }

 protected:
  std::string m_tag;
};

class TorusUpdateGeometryEvent : public TorusUpdateEvent {
 public:
  TorusUpdateGeometryEvent(const std::string& tag) : TorusUpdateEvent(tag) {}
  virtual inline const std::string name() const override {
    return "TorusUpdateGeometryEvent";
  }
};

class TorusUpdateTopologyEvent : public TorusUpdateEvent {
 public:
  TorusUpdateTopologyEvent(const std::string& tag) : TorusUpdateEvent(tag) {}
  virtual inline const std::string name() const override {
    return "TorusUpdateTopologyEvent";
  }
};

#endif  // MCAD_EVENTS_TORUS_EVENT_HH