#include "ui_layer.hh"
#include "cad_layer.hh"

UILayer::UILayer()
    : m_inner_radius(4.0f),
      m_outer_radius(6.0f),
      m_horizontal_density(8),
      m_vertical_density(8),
      m_color{1.0f, 1.0f, 0.0f} {}

void UILayer::configure() {}

void UILayer::update() {
  mge::UIManager::start_frame();

  auto size = ImGui::GetMainViewport()->Size;
  ImGui::SetNextWindowSize({size.x * 0.2f, size.y});
  ImGui::SetNextWindowPos({size.x * 0.8f, 0});
  ImGui::Begin("Parameters", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove);

  ImGui::Text("Torus");
  if (ImGui::InputFloat("inner radius", &m_inner_radius, 0.1f, 1.0f, "%.2f")) {
    m_inner_radius = std::clamp(m_inner_radius, 0.0f, m_outer_radius);
    auto event = TorusModifiedEvent(
        m_inner_radius, m_outer_radius, m_horizontal_density,
        m_vertical_density, glm::vec3{m_color[0], m_color[1], m_color[2]});
    m_send_event(event);
  }
  if (ImGui::InputFloat("outer radius", &m_outer_radius, 0.1f, 1.0f, "%.2f")) {
    m_outer_radius =
        m_outer_radius >= m_inner_radius ? m_outer_radius : m_inner_radius;
    auto event = TorusModifiedEvent(
        m_inner_radius, m_outer_radius, m_horizontal_density,
        m_vertical_density, glm::vec3{m_color[0], m_color[1], m_color[2]});
    m_send_event(event);
  }
  if (ImGui::InputInt("horizontal density", &m_horizontal_density, 1, 1)) {
    m_horizontal_density = std::clamp(m_horizontal_density, 3, 128);
    auto event = TorusModifiedEvent(
        m_inner_radius, m_outer_radius, m_horizontal_density,
        m_vertical_density, glm::vec3{m_color[0], m_color[1], m_color[2]});
    m_send_event(event);
  }
  if (ImGui::InputInt("vertical density", &m_vertical_density, 1, 1)) {
    m_vertical_density = std::clamp(m_vertical_density, 3, 128);
    auto event = TorusModifiedEvent(
        m_inner_radius, m_outer_radius, m_horizontal_density,
        m_vertical_density, glm::vec3{m_color[0], m_color[1], m_color[2]});
    m_send_event(event);
  };
  if (ImGui::ColorPicker3(
          "color", m_color,
          ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha)) {
    auto event = TorusModifiedEvent(
        m_inner_radius, m_outer_radius, m_horizontal_density,
        m_vertical_density, glm::vec3{m_color[0], m_color[1], m_color[2]});
    m_send_event(event);
  }

  ImGui::Text("Other");
  ImGuiIO& io = ImGui::GetIO();
  ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

  ImGui::End();
  ImGui::Render();
  mge::UIManager::end_frame();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UILayer::handle_event(mge::Event& event, float dt) {}