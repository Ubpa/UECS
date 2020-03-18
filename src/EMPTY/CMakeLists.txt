Ubpa_GlobGroupSrcs(RST sources PATHS
	${CMAKE_CURRENT_SOURCE_DIR}
	"${PROJECT_SOURCE_DIR}/include/UECS"
)

Ubpa_AddTarget(MODE "EXE" SOURCES ${sources} TEST ON LIBS Ubpa::UECS_core)
