Ubpa_AddTarget(
  MODE STATIC
  RET_TARGET_NAME tname
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UECS"
  LIB
    Ubpa::UTemplate_core
    Ubpa::UGraphviz_core
  INC
    "${PROJECT_SOURCE_DIR}/include"
)

target_precompile_headers(${tname} PRIVATE "${PROJECT_SOURCE_DIR}/include/UECS/World.h")
