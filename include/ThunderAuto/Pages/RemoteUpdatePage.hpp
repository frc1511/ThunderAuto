#pragma once

#include <ThunderAuto/DocumentManager.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/Page.hpp>
#include <networktables/NetworkTable.h>
#include <networktables/NetworkTableInstance.h>

class RemoteUpdatePage : public Page {
  const DocumentManager& m_documentManager;
  const DocumentEditManager& m_history;

  nt::NetworkTableInstance m_networkTableInstance;
  std::shared_ptr<nt::NetworkTable> m_thunderAutoNetworkTable;

  bool m_wasOnConnectionTab = true;

  bool m_startedClient = false;

  bool m_useCustomServerIP = false;
  char m_customServerIP[64] = "127.0.0.1";
  int m_teamNumber = 1511;
  bool m_driverStationRunning = true;

  bool m_wasUpdateSent = false;
  bool m_wasLastUpdateSentSuccessfully = false;

 public:
  RemoteUpdatePage(const DocumentManager& documentManager, const DocumentEditManager& history);
  ~RemoteUpdatePage();

  const char* name() const noexcept override { return "Remote Update"; }

  void present(bool* running) override;

 private:
  void presentConnectionTab();
  void presentUpdateTab();

  void sendUpdate();
};

