Ubpa_AddTarget(
  MODE STATIC
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UECS"
  LIB
    Ubpa::UTemplate_core
    Ubpa::UContainer_core
    Ubpa::UGraphviz_core
  INC
    "${PROJECT_SOURCE_DIR}/include"
)
