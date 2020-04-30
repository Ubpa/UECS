Ubpa_GetTargetName(core "${PROJECT_SOURCE_DIR}/src/core")
Ubpa_AddTarget(MODE "EXE" TEST ON LIBS "Ubpa::${core}")
