#ifndef MCAD_EVENTS_ADD_EVENT_HH
#define MCAD_EVENTS_ADD_EVENT_HH

#include "mge.hh"

class AddEvent : public mge::Event {
 public:
  AddEvent() {}
  virtual inline const std::string name() const override { return "AddEvent"; }
};

class AnnounceNewEntityEvent : public mge::Event {
 public:
  AnnounceNewEntityEvent(const std::string& tag) : m_tag(tag) {}
  virtual inline const std::string name() const override {
    return "AnnounceNewEntityEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }

 protected:
  const std::string& m_tag;
};

class AddTorusEvent : public AddEvent {
 public:
  AddTorusEvent() : AddEvent() {}
  virtual inline const std::string name() const override {
    return "AddTorusEvent";
  }
};

class AddPointEvent : public AddEvent {
 public:
  AddPointEvent() : AddEvent() {}
  virtual inline const std::string name() const override {
    return "AddPointEvent";
  }
};

#endif  // MCAD_EVENTS_ADD_EVENT_HH