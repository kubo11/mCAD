#ifndef MCAD_EVENTS_ADD_EVENT_HH
#define MCAD_EVENTS_ADD_EVENT_HH

#include "mge.hh"

class AddEvent : public mge::Event {
 public:
  AddEvent() {}
  virtual inline const std::string name() const override { return "AddEvent"; }
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

class AddBezierEvent : public AddEvent {
 public:
  AddBezierEvent() : AddEvent() {}
  virtual inline const std::string name() const override {
    return "AddBezierEvent";
  }
};

class NewEntityEvent : public mge::Event {
 public:
  NewEntityEvent(const std::string& tag) : m_tag(tag) {}
  virtual inline const std::string name() const override {
    return "NewEntityEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }

 private:
  std::string m_tag;
};

#endif  // MCAD_EVENTS_ADD_EVENT_HH