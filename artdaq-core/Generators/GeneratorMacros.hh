#ifndef artdaq_core_Generators_GeneratorMacros_hh
#define artdaq_core_Generators_GeneratorMacros_hh

#include "artdaq-core/Generators/FragmentGenerator.hh"
#include "fhiclcpp/fwd.h"

#include <memory>

namespace artdaq
{
	/**
   * \brief Constructs a FragmentGenerator instance, and returns a pointer to it
   * \param ps Parameter set for initializing the FragmentGenerator
   * \return A smart pointer to the FragmentGenerator
   */
	typedef std::unique_ptr<artdaq::FragmentGenerator> makeFunc_t(fhicl::ParameterSet const& ps);
}

#define DEFINE_ARTDAQ_GENERATOR(klass)                                \
	/** \brief Function exposed by plugin library to allow external   \
	* code to construct instances of klass.                           \
	* \param ps Parameter set for initializing the klass              \
	* \return A smart pointer to the klass                            \
	*/                                                                \
  extern "C"                                                          \
  std::unique_ptr<artdaq::FragmentGenerator>                          \
  make(fhicl::ParameterSet const & ps) {                              \
	return std::unique_ptr<artdaq::FragmentGenerator>(new klass(ps)); \
  }

#endif /* artdaq_core_Generators_GeneratorMacros_hh */
