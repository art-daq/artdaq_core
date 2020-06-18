
#define TRACE_NAME "ExceptionHandler"
#include "ExceptionHandler.hh"
#include "ExceptionStackTrace.hh"

#include "canvas/Utilities/Exception.h"
#include "cetlib_except/exception.h"
#include "tracemf.h"

#include <boost/exception/all.hpp>
namespace artdaq {

#ifdef EXCEPTIONSTACKTRACE
inline void PrintExceptionStackTrace()
{
	auto message = artdaq::debug::getStackTraceCollector().print_stacktrace();

	std::string::size_type pos = 0;
	std::string::size_type prev = 0;

	while ((pos = message.find('\n', prev)) != std::string::npos)
	{
		TLOG(TLVL_DEBUG) << message.substr(prev, pos - prev);
		prev = pos + 1;
	}

	TLOG(TLVL_DEBUG) << message.substr(prev);
}
#else
inline void PrintExceptionStackTrace()
{}
#endif

void ExceptionHandler(ExceptionHandlerRethrow decision, std::string optional_message)
{
	if (optional_message != "")
	{
		TLOG(TLVL_ERROR) << optional_message;
	}

	try
	{
		throw;
	}
	catch (const art::Exception& e)
	{
		TLOG(TLVL_ERROR) << "art::Exception object caught:"
		                 << " returnCode = " << e.returnCode() << ", categoryCode = " << e.categoryCode() << ", category = " << e.category();
		TLOG(TLVL_ERROR) << "art::Exception object stream:" << e;
		PrintExceptionStackTrace();

		if (decision == ExceptionHandlerRethrow::yes) { throw; }
	}
	catch (const cet::exception& e)
	{
		TLOG(TLVL_ERROR) << "cet::exception object caught:" << e.explain_self();
		PrintExceptionStackTrace();

		if (decision == ExceptionHandlerRethrow::yes) { throw; }
	}
	catch (const boost::exception& e)
	{
		TLOG(TLVL_ERROR) << "boost::exception object caught: " << boost::diagnostic_information(e);
		PrintExceptionStackTrace();

		if (decision == ExceptionHandlerRethrow::yes) { throw; }
	}
	catch (const std::exception& e)
	{
		TLOG(TLVL_ERROR) << "std::exception caught: " << e.what();
		PrintExceptionStackTrace();

		if (decision == ExceptionHandlerRethrow::yes) { throw; }
	}
	catch (...)
	{
		TLOG(TLVL_ERROR) << "Exception of type unknown to artdaq::ExceptionHandler caught";
		PrintExceptionStackTrace();

		if (decision == ExceptionHandlerRethrow::yes) { throw; }
	}
}
}  // namespace artdaq
