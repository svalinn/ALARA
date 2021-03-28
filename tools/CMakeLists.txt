add_executable(dant2alara dant2alara.C)
install(TARGETS dant2alara DESTINATION bin)
install(PROGRAMS extract_pathways summary DESTINATION bin)
