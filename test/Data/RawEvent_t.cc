#include "artdaq-core/Data/RawEvent.hh"
#include "artdaq-core/Data/Fragment.hh"

#define BOOST_TEST_MODULE(RawEvent_t)
#include "cetlib/quiet_unit_test.hpp"

BOOST_AUTO_TEST_SUITE(RawEvent_test)

BOOST_AUTO_TEST_CASE(InsertFragment)
{
  // SCF - The RawEvent::insertFragment() method used to check and verify that
  // the sequence ID of the fragment equaled the sequence ID in the RawEvent
  // header.  This doesn't work for the DS50 aggregator as it packs multiple
  // fragments with different sequence IDs into a single RawEvent.  This test
  // verifies that the we're able to do this.
  artdaq::RawEvent r1(1, 1, 1);
  std::unique_ptr<artdaq::Fragment> f1(new artdaq::Fragment(1, 1));
  std::unique_ptr<artdaq::Fragment> f2(new artdaq::Fragment(2, 1));
  std::unique_ptr<artdaq::Fragment> f3(new artdaq::Fragment(3, 1));

  try {
    r1.insertFragment(std::move(f1));
    r1.insertFragment(std::move(f2));
    r1.insertFragment(std::move(f3));
    //BOOST_REQUIRE_EQUAL(r1.numFragments(), 3);
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should not have thrown exception");
  }
}

BOOST_AUTO_TEST_SUITE_END()

