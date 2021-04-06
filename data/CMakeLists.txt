macro(create_symlink filename sym_name)
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} \
                -E create_symlink \ ${filename} ${sym_name} \
                WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/data)")
endmacro ()

# Data files
file(GLOB DATA_FILES "*")
list(REMOVE_ITEM DATA_FILES ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt)
list(REMOVE_ITEM DATA_FILES ${CMAKE_CURRENT_LIST_DIR}/Makefile.am   )

# Install data files
install(FILES ${DATA_FILES} DESTINATION data)

# Create symlinks
create_symlink(ANS6_4_3.txt                   ANS6.4.3                 )
create_symlink(ANS6_4_3.txt                   ANS6_4_3                 )
create_symlink(elelib.std                     elelib_std               )
create_symlink(EU.clearance.Bq_kg             EU.clearance             )
create_symlink(EU.clearance.Bq_kg             EU_clearance             )
create_symlink(EU.clearance.2000.Bq_kg        EU.clearance.Bq_kg       )
create_symlink(FetterC_hi.Ci_m3.wdr           FetterHi                 )
create_symlink(FetterC_lo.Ci_m3.wdr           FetterLo                 )
create_symlink(IAEA.clearance.Bq_kg           IAEA.clearance           )
create_symlink(IAEA.clearance.Bq_kg           IAEA_clearance           )
create_symlink(IAEA.clearance.2004.Bq_kg      IAEA.clearance.Bq_kg     )
create_symlink(NRCA.Ci_m3.wdr                 NRCA                     )
create_symlink(NRCC.Ci_m3.wdr                 NRCC                     )
create_symlink(NRC.clearance.steel.Bq_kg      NRC.clearance.steel      )
create_symlink(NRC.clearance.steel.Bq_kg      NRC_clearance_steel      )
create_symlink(NRC.clearance.steel.2003.Bq_kg NRC.clearance.steel.Bq_kg)
