Ubpa_GlobGroupSrcs(RST sources PATHS
	${CMAKE_CURRENT_SOURCE_DIR}
	"${PROJECT_SOURCE_DIR}/include/UECS"
)

Ubpa_AddTarget_GDR(MODE "LIB"
	SOURCES ${sources}
	INTERFACE_INC ON
	INC_DIRS "_deps"
	PUBLIC_GLIBS Ubpa::UTemplate_core Ubpa::UBL_core
)
