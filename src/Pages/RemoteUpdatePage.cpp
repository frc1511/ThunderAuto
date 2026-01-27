#include <ThunderAuto/Pages/RemoteUpdatePage.hpp>

#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>

#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/ImGuiScopedField.hpp>
#include <IconsLucide.h>
#include <imgui_raii.h>
#include <ctime>

RemoteUpdatePage::RemoteUpdatePage(const DocumentManager& documentManager, const DocumentEditManager& history)
    : m_documentManager(documentManager), m_history(history) {
  m_networkTableInstance = nt::NetworkTableInstance::GetDefault();
  m_thunderAutoNetworkTable = m_networkTableInstance.GetTable("ThunderAuto");
}

RemoteUpdatePage::~RemoteUpdatePage() {
  if (m_startedClient) {
    m_networkTableInstance.StopClient();
  }
}

void RemoteUpdatePage::present(bool* running) {
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(REMOTE_UPDATE_PAGE_START_WIDTH), GET_UISIZE(REMOTE_UPDATE_PAGE_START_HEIGHT)),
      ImGuiCond_FirstUseEver);
  ImGui::Scoped scopedWindow = ImGui::Scoped::Window(name(), running);
  if (!scopedWindow || (running && !*running))
    return;

  auto scopedTabBar = ImGui::Scoped::TabBar("Remote Update Tabs");

  if (auto scopedTabItem = ImGui::Scoped::TabItem("Connection")) {
    presentConnectionTab();
  }

  if (auto scopedTabItem = ImGui::Scoped::TabItem("Update")) {
    presentUpdateTab();
  }
}

void RemoteUpdatePage::presentConnectionTab() {
  m_wasOnConnectionTab = true;

  {
    auto scopedField = ImGui::ScopedField::Builder("Custom IP").build();
    ImGui::Checkbox("##Custom IP", &m_useCustomServerIP);
  }

  ImGui::Separator();

  if (m_useCustomServerIP) {
    auto scopedField = ImGui::ScopedField::Builder("Server IP Address").build();

    ImGui::InputText("##Server IP Address", &m_customServerIP[0], sizeof(m_customServerIP));
  } else {
    {
      auto scopedField = ImGui::ScopedField::Builder("Team Number").build();
      ImGui::InputInt("##Team Number", &m_teamNumber);
    }

    {
      auto scopedField = ImGui::ScopedField::Builder("DS Running")
                             .tooltip("Is the FRC Driver Station currently running on this computer?")
                             .build();
      ImGui::Checkbox("##Driver Station Running", &m_driverStationRunning);
    }
  }
}

void RemoteUpdatePage::presentUpdateTab() {
  if (m_wasOnConnectionTab) {
    m_wasOnConnectionTab = false;

    if (!m_startedClient) {
      m_startedClient = true;
      m_networkTableInstance.StartClient4("ThunderAuto");
    }

    if (m_useCustomServerIP) {
      m_networkTableInstance.SetServer(m_customServerIP);
    } else {
      m_networkTableInstance.SetServerTeam(m_teamNumber);
      if (m_driverStationRunning) {
        m_networkTableInstance.StartDSClient();
      }
    }
  }

  bool isConnected = m_networkTableInstance.IsConnected();
  if (isConnected) {
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
  } else {
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not connected");
  }

  ImGui::Separator();

  {
    auto scopedDisabled = ImGui::Scoped::Disabled(!isConnected);

    ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, 0.f);
    if (ImGui::Button(ICON_LC_UPLOAD "  Send Update", buttonSize)) {
      sendUpdate();
    }
  }

  if (m_wasUpdateSent) {
    ImGui::Text("Last update status:");
    ImGui::SameLine();
    if (m_wasLastUpdateSentSuccessfully) {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Successfully published");
    } else {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to publish");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Check logs for more information");
      }
    }
  }
}

void RemoteUpdatePage::sendUpdate() {
  m_wasUpdateSent = true;

  const ThunderAutoProjectState& state = m_history.currentState();

  const std::vector<uint8_t> serializedData = SerializeThunderAutoProjectStateForTransmission(state);
  std::span<const uint8_t> serializedDataSpan(serializedData.data(), serializedData.size());

  const std::string& projectName = m_documentManager.name();

  bool res = m_thunderAutoNetworkTable->PutRaw(projectName, serializedDataSpan);
  if (!res) {
    ThunderAutoLogger::Error("Failed to publish project update to NetworkTables");
    m_wasLastUpdateSentSuccessfully = false;
    return;
  }

  time_t now = std::time(nullptr);
  std::tm* localTime = std::localtime(&now);

  std::string timestamp =
      fmt::format("{:04}-{:02}-{:02}_{:02}-{:02}-{:02}", localTime->tm_year + 1900, localTime->tm_mon + 1,
                  localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);

  res = m_thunderAutoNetworkTable->GetSubTable("Timestamps")->PutString(projectName, timestamp);
  m_wasLastUpdateSentSuccessfully = res;

  if (res) {
    ThunderAutoLogger::Info("Successfully published project update to NetworkTables");
  } else {
    ThunderAutoLogger::Error("Failed to publish project update timestamp to NetworkTables");
  }
}

