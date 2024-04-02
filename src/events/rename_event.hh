#ifndef MCAD_EVENTS_RENAME_EVENT_HH
#define MCAD_EVENTS_RENAME_EVENT_HH

#include "mge.hh"

class RenameEvent : public mge::Event {
 public:
  RenameEvent(const std::string& old_tag, const std::string& new_tag)
      : m_old_tag(old_tag), m_new_tag(new_tag), m_status{} {}
  virtual inline const std::string name() const override {
    return "RenameEvent";
  }
  inline const std::string& get_old_tag() const { return m_old_tag; }
  inline const std::string& get_new_tag() const { return m_new_tag; }
  inline bool get_status() const { return m_status; }
  inline void set_status(bool status) { m_status = status; }

 protected:
  std::string m_old_tag;
  std::string m_new_tag;
  bool m_status;
};

#endif  // MCAD_EVENTS_RENAME_EVENT_HH