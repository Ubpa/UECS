Ubpa_GetTargetName(cmpt "${PROJECT_SOURCE_DIR}/src/cmpt")
Ubpa_AddTarget(MODE "EXE" LIBS ${cmpt} TEST ON)
