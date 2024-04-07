#ifndef MCAD_EVENTS_TRANSFORM_EVENT_HH
#define MCAD_EVENTS_TRANSFORM_EVENT_HH

#include "mge.hh"

class MoveByCursorEvent : public mge::Event {
 public:
  MoveByCursorEvent() {}
  virtual inline const std::string name() const override {
    return "MoveByCursorEvent";
  }
};

class MoveByUIEvent : public mge::Event {
 public:
  MoveByUIEvent(const glm::vec3& offset) : m_offset(offset) {}
  virtual inline const std::string name() const override {
    return "MoveByUIEvent";
  }

  inline const glm::vec3& get_offset() const { return m_offset; }

 private:
  glm::vec3 m_offset;
};

class ScaleByCursorEvent : public mge::Event {
 public:
  ScaleByCursorEvent(glm::vec2 beg, glm::vec2 end) : m_beg(beg), m_end(end) {}
  virtual inline const std::string name() const override {
    return "ScaleByCursorEvent";
  }
  inline glm::vec2 get_beg() const { return m_beg; }
  inline glm::vec2 get_end() const { return m_end; }

 protected:
  glm::vec2 m_beg;
  glm::vec2 m_end;
};

class ScaleByUIEvent : public mge::Event {
 public:
  ScaleByUIEvent(const glm::vec3& scale) : m_scale(scale) {}
  virtual inline const std::string name() const override {
    return "ScaleByUIEvent";
  }

  inline const glm::vec3& get_scale() const { return m_scale; }

 private:
  glm::vec3 m_scale;
};

class RotateByCursorEvent : public mge::Event {
 public:
  RotateByCursorEvent(glm::vec2 beg, glm::vec2 end, glm::vec3 axis)
      : m_beg(beg), m_end(end), m_axis(axis) {}
  virtual inline const std::string name() const override {
    return "RotateByCursorEvent";
  }
  inline glm::vec2 get_beg() const { return m_beg; }
  inline glm::vec2 get_end() const { return m_end; }
  inline glm::vec3 get_axis() const { return m_axis; }

 protected:
  glm::vec2 m_beg;
  glm::vec2 m_end;
  glm::vec3 m_axis;
};

class RotateByUIEvent : public mge::Event {
 public:
  RotateByUIEvent(const mge::quat& rotation) : m_rotation(rotation) {}
  virtual inline const std::string name() const override {
    return "RotateByUIEvent";
  }

  inline const mge::quat get_rotation() const { return m_rotation; }

 private:
  mge::quat m_rotation;
};

#endif  // MCAD_EVENTS_TRANSFORM_EVENT_HH