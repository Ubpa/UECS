Ubpa_AddTarget(
  TEST
  RET_TARGET_NAME tname
  MODE EXE
  LIB
    Ubpa::UECS_core
)
target_precompile_headers(${tname} REUSE_FROM UECS_core)
