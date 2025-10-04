set(THUNDERAUTO_POPUP_DIR "${THUNDERAUTO_SRC_DIR}/Popups")

add_thunder_auto_sources(
  "${THUNDERAUTO_POPUP_DIR}/NewProjectPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/UnsavedPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/NewFieldPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/WelcomePopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/OpenProjectErrorPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/SaveProjectErrorPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/ProjectVersionPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/CSVExportPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/NewTrajectoryPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/RenameTrajectoryPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/DuplicateTrajectoryPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/LinkTrajectoryPointPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/NewActionPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/RenameActionPopup.cpp"
  "${THUNDERAUTO_POPUP_DIR}/RecursiveActionErrorPopup.cpp"
)

