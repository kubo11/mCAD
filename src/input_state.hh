#ifndef MCAD_INPUT_STATE_HH
#define MCAD_INPUT_STATE_HH

#define INPUT_STATE_SIZE 7

enum class InputState : int {
  SELECT = 1 << 0,
  DELETE = 1 << 1,
  MOVE = 1 << 2,
  SCALE = 1 << 3,
  ROTATEX = 1 << 4,
  ROTATEY = 1 << 5,
  ROTATEZ = 1 << 6,
};

inline std::string to_string(InputState state) {
  switch (state) {
    case InputState::SELECT:
      return "select";
    case InputState::DELETE:
      return "delete";
    case InputState::MOVE:
      return "move";
    case InputState::SCALE:
      return "scale";
    case InputState::ROTATEX:
      return "rotate x";
    case InputState::ROTATEY:
      return "rotate y";
    case InputState::ROTATEZ:
      return "rotate z";

    default:
      return "invalid";
  }
};

#endif  // MCAD_INPUT_STATE_HH