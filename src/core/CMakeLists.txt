Ubpa_GlobGroupSrcs(RST sources PATHS
	${CMAKE_CURRENT_SOURCE_DIR}
	"${PROJECT_SOURCE_DIR}/include/UECS"
)

Ubpa_ToPackageName(utemplate_package_name UTemplate ${UTemplate_VERSION})

Ubpa_AddTarget(MODE "LIB"
	SOURCES ${sources}
	INTERFACE_INC ON
	LIBS "Ubpa::${utemplate_package_name}_core"
)
