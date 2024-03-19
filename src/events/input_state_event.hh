#ifndef MCAD_EVENTS_INPUT_STATE_EVENT_HH
#define MCAD_EVENTS_INPUT_STATE_EVENT_HH

#include "mge.hh"

#include "../input_state.hh"

class InputStateChangedEvent : public mge::Event {
 public:
  InputStateChangedEvent(const InputState& state) : m_state(state) {}
  virtual inline const std::string name() const override {
    return "InputStateChangedEvent";
  }
  inline const InputState& get_input_state() const { return m_state; }

 protected:
  InputState m_state;
};

#endif  // MCAD_EVENTS_INPUT_STATE_EVENT_HH