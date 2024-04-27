#ifndef MCAD_EVENTS_BEZIER_EVENT_H
#define MCAD_EVENTS_BEZIER_EVENT_H

#include "mge.hh"

class ShowBerensteinPolygonEvent : public mge::Event {
 public:
  ShowBerensteinPolygonEvent(const std::string& tag, bool show)
      : m_tag(tag), m_show(show) {}
  virtual inline const std::string name() const override {
    return "ShowBerensteinPolygonEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }
  inline bool get_status() const { return m_show; }

 private:
  std::string m_tag;
  bool m_show;
};

class AddControlPointByTagEvent : public mge::Event {
 public:
  AddControlPointByTagEvent(const std::string& bezier_tag,
                            const std::string& point_tag)
      : m_bezier_tag(bezier_tag), m_point_tag(point_tag) {}
  virtual inline const std::string name() const override {
    return "AddControlPointByTagEvent";
  }
  inline const std::string& get_bezier_tag() const { return m_bezier_tag; }
  inline const std::string& get_point_tag() const { return m_point_tag; }

 private:
  std::string m_bezier_tag;
  std::string m_point_tag;
};

class RemoveControlPointByTagEvent : public mge::Event {
 public:
  RemoveControlPointByTagEvent(const std::string& bezier_tag,
                               const std::string& point_tag)
      : m_bezier_tag(bezier_tag), m_point_tag(point_tag) {}
  virtual inline const std::string name() const override {
    return "RemoveControlPointByTagEvent";
  }
  inline const std::string& get_bezier_tag() const { return m_bezier_tag; }
  inline const std::string& get_point_tag() const { return m_point_tag; }

 private:
  std::string m_bezier_tag;
  std::string m_point_tag;
};

class AddControlPointByPositionEvent : public mge::Event {
 public:
  AddControlPointByPositionEvent(const std::string& bezier_tag,
                                 const glm::vec2& position)
      : m_bezier_tag(bezier_tag), m_position(position) {}
  virtual inline const std::string name() const override {
    return "AddControlPointByPositionEvent";
  }
  inline const std::string& get_bezier_tag() const { return m_bezier_tag; }
  inline const glm::vec2& get_position() const { return m_position; }

 private:
  std::string m_bezier_tag;
  glm::vec2 m_position;
};

class RemoveControlPointByPositionEvent : public mge::Event {
 public:
  RemoveControlPointByPositionEvent(const std::string& bezier_tag,
                                    const glm::vec2& position)
      : m_bezier_tag(bezier_tag), m_position(position) {}
  virtual inline const std::string name() const override {
    return "RemoveControlPointByPositionEvent";
  }
  inline const std::string& get_bezier_tag() const { return m_bezier_tag; }
  inline const glm::vec2& get_position() const { return m_position; }

 private:
  std::string m_bezier_tag;
  glm::vec2 m_position;
};

#endif  // MCAD_EVENTS_BEZIER_EVENT_H
