#ifndef artdaq_core_Data_detail_RawFragmentHeader_hh
#define artdaq_core_Data_detail_RawFragmentHeader_hh
// detail::RawFragmentHeader is an overlay that provides the user's view
// of the data contained within a Fragment. It is intended to be hidden
// from the user of Fragment, as an implementation detail. The interface
// of Fragment is intended to be used to access the data.

#include <cstddef>
#include "artdaq-core/Data/dictionarycontrol.hh"
#include "cetlib/exception.h"

extern "C" {
#include <stdint.h>
}

namespace artdaq {
  namespace detail {
    struct RawFragmentHeader;
  }
}

struct artdaq::detail::RawFragmentHeader {
  typedef unsigned long long RawDataType;

#if HIDE_FROM_ROOT
  typedef uint16_t version_t;
  typedef uint64_t sequence_id_t;
  typedef uint8_t  type_t;
  typedef uint16_t fragment_id_t;
  typedef uint8_t  metadata_word_count_t;
  typedef uint32_t timestamp_t;

  // define special values for type_t
  static constexpr type_t INVALID_TYPE = 0;
  static constexpr type_t FIRST_USER_TYPE = 1;
  static constexpr type_t LAST_USER_TYPE = 224;
  static constexpr type_t FIRST_SYSTEM_TYPE = 225;
  static constexpr type_t LAST_SYSTEM_TYPE = 255;
  static constexpr type_t InvalidFragmentType = INVALID_TYPE;
  static constexpr type_t EndOfDataFragmentType = FIRST_SYSTEM_TYPE;
  static constexpr type_t DataFragmentType = FIRST_SYSTEM_TYPE+1;
  static constexpr type_t InitFragmentType = FIRST_SYSTEM_TYPE+2;
  static constexpr type_t EndOfRunFragmentType = FIRST_SYSTEM_TYPE+3;
  static constexpr type_t EndOfSubrunFragmentType = FIRST_SYSTEM_TYPE+4;
  static constexpr type_t ShutdownFragmentType = FIRST_SYSTEM_TYPE+5;
  static constexpr type_t EmptyFragmentType = FIRST_SYSTEM_TYPE+6;
  static constexpr type_t ContainerFragmentType = FIRST_SYSTEM_TYPE+7;

  // Each of the following invalid values is chosen based on the
  // size of the bitfield in which the corresponding data are
  // encoded; if any of the sizes are changed, the corresponding
  // values must be updated.
  static const version_t InvalidVersion  = 0xFFFF;
  static const sequence_id_t InvalidSequenceID = 0xFFFFFFFFFFFF;
  static const fragment_id_t InvalidFragmentID = 0xFFFF;
  static const timestamp_t InvalidTimestamp = 0xFFFFFFFF;

  RawDataType word_count          : 32; // number of RawDataTypes in this Fragment
  RawDataType version             : 16;
  RawDataType type                :  8;
  RawDataType metadata_word_count :  8;

  RawDataType sequence_id : 48;
  RawDataType fragment_id : 16;
  RawDataType timestamp   : 32;

  // 27-Feb-2013, KAB - As we discussed recently, we will go ahead
  // and reserve another longword for future needs.  The choice of
  // four 16-bit values is arbitrary and will most certainly change
  // once we identify the future needs.
  RawDataType unused1     : 16;
  RawDataType unused2     : 16;

  constexpr static std::size_t num_words();

  void setUserType(uint8_t utype);
  void setSystemType(uint8_t stype);

#endif /* HIDE_FROM_ROOT */

};

#if HIDE_FROM_ROOT
inline
constexpr
std::size_t
artdaq::detail::RawFragmentHeader::num_words()
{
  return sizeof(detail::RawFragmentHeader) / sizeof(RawDataType);
}

// Compile-time check that the assumption made in num_words() above is
// actually true.
static_assert((artdaq::detail::RawFragmentHeader::num_words() *
               sizeof(artdaq::detail::RawFragmentHeader::RawDataType)) ==
              sizeof(artdaq::detail::RawFragmentHeader),
              "sizeof(RawFragmentHeader) is not an integer "
              "multiple of sizeof(RawDataType)!");

inline
void
artdaq::detail::RawFragmentHeader::setUserType(uint8_t utype)
{
  if (utype < FIRST_USER_TYPE || utype > LAST_USER_TYPE) {
    throw cet::exception("InvalidValue")
      << "RawFragmentHeader user types must be in the range of "
      << ((int)FIRST_USER_TYPE) << " to " << ((int)LAST_USER_TYPE);
  }
  type = utype;
}

inline
void
artdaq::detail::RawFragmentHeader::setSystemType(uint8_t stype)
{
  if (stype < FIRST_SYSTEM_TYPE /*|| stype > LAST_SYSTEM_TYPE*/) {
    throw cet::exception("InvalidValue")
      << "RawFragmentHeader system types must be in the range of "
      << ((int)FIRST_SYSTEM_TYPE) << " to " << ((int)LAST_SYSTEM_TYPE);
  }
  type = stype;
}
#endif

#endif /* artdaq_core_Data_detail_RawFragmentHeader_hh */
