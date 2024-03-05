#include "ui_layer.hh"
#include "ellipsoid_layer.hh"

UILayer::UILayer()
    : m_a(0.5f),
      m_b(1.0f),
      m_c(1.5f),
      m_color{1.0f, 1.0f, 0.0f},
      m_ambient(0.1f),
      m_diffuse(0.5f),
      m_specular(0.5f),
      m_shininess(0.5f),
      m_accuracy(5) {}

void UILayer::configure() {}

void UILayer::update() {
  mge::UIManager::start_frame();

  auto size = ImGui::GetMainViewport()->Size;
  ImGui::SetNextWindowSize({size.x * 0.2f, size.y});
  ImGui::SetNextWindowPos({size.x * 0.8f, 0});
  ImGui::Begin("Parameters", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove);

  ImGui::Text("Ellipsoid");
  if (ImGui::InputFloat("a", &m_a, 0.1f, 1.0f, "%.2f")) {
    auto event = EllipsoidModifiedEvent(generate_ellipsoid());
    m_send_event(event);
  }
  if (ImGui::InputFloat("b", &m_b, 0.1f, 1.0f, "%.2f")) {
    auto event = EllipsoidModifiedEvent(generate_ellipsoid());
    m_send_event(event);
  }
  if (ImGui::InputFloat("c", &m_c, 0.1f, 1.0f, "%.2f")) {
    auto event = EllipsoidModifiedEvent(generate_ellipsoid());
    m_send_event(event);
  }
  if (ImGui::ColorPicker3(
          "color", m_color,
          ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha)) {
    auto event = EllipsoidModifiedEvent(generate_ellipsoid());
    m_send_event(event);
  }
  if (ImGui::InputFloat("ambient", &m_ambient, 0.1f, 1.0f, "%.2f")) {
    auto event = EllipsoidModifiedEvent(generate_ellipsoid());
    m_send_event(event);
  }
  if (ImGui::InputFloat("diffuse", &m_diffuse, 0.1f, 1.0f, "%.2f")) {
    auto event = EllipsoidModifiedEvent(generate_ellipsoid());
    m_send_event(event);
  }
  if (ImGui::InputFloat("specular", &m_specular, 0.1f, 1.0f, "%.2f")) {
    auto event = EllipsoidModifiedEvent(generate_ellipsoid());
    m_send_event(event);
  }
  if (ImGui::InputFloat("shininess", &m_shininess, 0.1f, 1.0f, "%.2f")) {
    auto event = EllipsoidModifiedEvent(generate_ellipsoid());
    m_send_event(event);
  }

  ImGui::Text("Other");
  if (ImGui::InputInt("accuracy", &m_accuracy, 1, 1)) {
    m_accuracy = std::clamp(m_accuracy, 0, 7);
    auto event = mge::RaycasterUpdateMaxPixelSizeExponentEvent(m_accuracy);
    m_send_event(event);
  };

  ImGuiIO& io = ImGui::GetIO();
  ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

  ImGui::End();
  ImGui::Render();
  mge::UIManager::end_frame();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UILayer::handle_event(mge::Event& event, float dt) {}