Ubpa_GlobGroupSrcs(RST sources PATHS
	${CMAKE_CURRENT_SOURCE_DIR}
	"${PROJECT_SOURCE_DIR}/include/UECS/cmpt"
)

Ubpa_AddTarget(MODE "LIB"
	SOURCES ${sources}
	INTERFACE_INC ON
	LIBS Ubpa::UECS_core
)
