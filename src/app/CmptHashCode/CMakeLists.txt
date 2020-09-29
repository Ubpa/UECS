Ubpa_GetTargetName(core "${PROJECT_SOURCE_DIR}/src/core")
Ubpa_AddTarget(
  MODE EXE
  LIB
    Ubpa::UTemplate_core
  INC_PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)
