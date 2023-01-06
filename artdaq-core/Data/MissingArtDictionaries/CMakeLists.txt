#art_dictionary()
if(DEFINED ENV{USE_ASAN} OR DEFINED ENV{USE_TSAN})
build_dictionary( NO_CHECK_CLASS_VERSION
  DICTIONARY_LIBRARIES
  artdaq-core_Data
  artdaq-core_Data_ParentageMap
  canvas::canvas
)
else()
build_dictionary(
  DICTIONARY_LIBRARIES
  artdaq-core_Data
  artdaq-core_Data_ParentageMap
  canvas::canvas
)
endif()

disable_sanitizers(artdaq-core_Data_MissingArtDictionaries_dict)

install_headers()
install_source()
