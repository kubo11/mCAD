#ifndef MCAD_EVENTS_INPUT_STATE_EVENT
#define MCAD_EVENTS_INPUT_STATE_EVENT

#include "mge.hh"

#include "../input_state.hh"

enum class InputStateEvents { Update };

class InputStateUpdateEvent : public mge::Event<InputStateEvents> {
 public:
  InputStateUpdateEvent(InputState state)
      : mge::Event<InputStateEvents>(InputStateEvents::Update, "InputStateUpdateEvent"), state(state) {}

  InputState state;
};

#endif  // MCAD_EVENTS_INPUT_STATE_EVENT