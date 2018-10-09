#ifndef artdaq_core_Data_ContainerFragment_hh
#define artdaq_core_Data_ContainerFragment_hh

#include "artdaq-core/Data/Fragment.hh"
#include "cetlib_except/exception.h"

//#include <ostream>
//#include <vector>

// Implementation of "ContainerFragment", an artdaq::Fragment overlay class

namespace artdaq
{
	class ContainerFragment;
}

/**
 * \brief The artdaq::ContainerFragment class represents a Fragment which contains other Fragments
 */
class artdaq::ContainerFragment
{
public:

	static constexpr uint8_t CURRENT_VERSION = 1;
	static constexpr size_t CONTAINER_MAGIC = 0x00BADDEED5B1BEE5;

	/**
	 * \brief Contains the information necessary for retrieving Fragment objects from the ContainerFragment
	 */
	struct MetadataV0
	{
		/**
		 * \brief The maximum capacity of the ContainerFragment (in fragments)
		 */
		static constexpr int CONTAINER_FRAGMENT_COUNT_MAX = 100;

		typedef uint8_t data_t; ///< Basic unit of data-retrieval
		typedef uint64_t count_t; ///< Size of block_count variables

		count_t block_count : 55; ///< The number of Fragment objects stored in the ContainerFragment
		count_t fragment_type : 8; ///< The Fragment::type_t of stored Fragment objects
		count_t missing_data : 1; ///< Flag if the ContainerFragment knows that it is missing data

		/// Offset of each Fragment within the ContainerFragment
		size_t index[CONTAINER_FRAGMENT_COUNT_MAX];

		/// Size of the Metadata object
		static size_t const size_words = 8ul + CONTAINER_FRAGMENT_COUNT_MAX * sizeof(size_t) / sizeof(data_t); // Units of Header::data_t
	};
	static_assert (sizeof(MetadataV0) == MetadataV0::size_words * sizeof(MetadataV0::data_t), "ContainerFragment::MetadataV0 size changed");

	/**
	 * \brief Contains the information necessary for retrieving Fragment objects from the ContainerFragment
	 */
	struct Metadata
	{
		typedef uint8_t data_t; ///< Basic unit of data-retrieval
		typedef uint64_t count_t; ///< Size of block_count variables

		count_t block_count : 16; ///< The number of Fragment objects stored in the ContainerFragment
		count_t fragment_type : 8; ///< The Fragment::type_t of stored Fragment objects
		count_t version : 4;
		count_t missing_data : 1; ///< Flag if the ContainerFragment knows that it is missing data
		count_t has_index : 1;
		count_t unused_flag1 : 1;
		count_t unused_flag2 : 1;
		count_t unused : 32;

		uint64_t index_offset;
				
		/// Size of the Metadata object
		static size_t const size_words = 16ul; // Units of Header::data_t
	};
	static_assert (sizeof(Metadata) == Metadata::size_words * sizeof(Metadata::data_t), "ContainerFragment::Metadata size changed");

	Metadata const* UpgradeMetadata(MetadataV0 const* in) const
	{
		TLOG(TLVL_DEBUG, "ContainerFragment") << "Upgrading ContainerFragment::MetadataV0 into new ContainerFragment::Metadata";
		assert(in->block_count < std::numeric_limits<Metadata::count_t>::max());
		metadata_alloc_ = true;
		Metadata md;
		md.block_count = in->block_count;
		md.fragment_type = in->fragment_type;
		md.has_index = 0;
		md.missing_data = in->missing_data;
		md.version = 0;
		index_ptr_ = in->index;
		metadata_ = new Metadata(md);
		return metadata_;
	}

	/**
	 * \param f The Fragment object to use for data storage
	 * 
	 * The constructor simply sets its const private member "artdaq_Fragment_"
	 * to refer to the artdaq::Fragment object
	*/
	explicit ContainerFragment(Fragment const& f) : artdaq_Fragment_(f), index_ptr_(nullptr), index_alloc_(false),metadata_(nullptr), metadata_alloc_(false) { }

	virtual ~ContainerFragment()
	{
		if (index_alloc_)
		{
			delete[] index_ptr_;
		}
		if (metadata_alloc_)
		{
			delete metadata_;
		}
	}

	/**
	 * \brief const getter function for the Metadata
	 * \return const pointer to the Metadata
	 */
	Metadata const* metadata() const 
	{ 
		if (metadata_alloc_) return metadata_;

		if (artdaq_Fragment_.sizeBytes() - artdaq_Fragment_.dataSizeBytes() - sizeof(detail::RawFragmentHeader) == sizeof(MetadataV0))
		{
			return UpgradeMetadata(artdaq_Fragment_.metadata<MetadataV0>());
		}

		return artdaq_Fragment_.metadata<Metadata>();
	}
	
	/**
	 * \brief Gets the number of fragments stored in the ContainerFragment
	 * \return The number of Fragment objects stored in the ContainerFragment
	 */
	Metadata::count_t block_count() const { return metadata()->block_count; }
	/**
	 * \brief Get the Fragment::type_t of stored Fragment objects
	 * \return The Fragment::type_t of stored Fragment objects
	 */
	Fragment::type_t fragment_type() const { return static_cast<Fragment::type_t>(metadata()->fragment_type); }
	/**
	 * \brief Gets the flag if the ContainerFragment knows that it is missing data
	 * \return The flag if the ContainerFragment knows that it is missing data
	 */
	bool missing_data() const { return static_cast<bool>(metadata()->missing_data); }

	/**
	 * \brief Gets the start of the data
	 * \return Pointer to the first Fragment in the ContainerFragment
	 */
	void const* dataBegin() const
	{
		return reinterpret_cast<void const *>(&*artdaq_Fragment_.dataBegin());
	}

	/**
	 * \brief Gets the last Fragment in the ContainerFragment
	 * \return Pointer to the last Fragment in the ContainerFragment
	 */
	void const* dataEnd() const
	{
		return reinterpret_cast<void const *>(reinterpret_cast<uint8_t const *>(dataBegin()) + lastFragmentIndex());
	}

	/**
	 * \brief Gets a specific Fragment from the ContainerFragment
	 * \param index The Fragment index to return
	 * \return Pointer to the specified Fragment in the ContainerFragment
	 * \exception cet::exception if the index is out-of-range
	 */
	FragmentPtr at(size_t index) const
	{
		if (index >= block_count() || block_count() == 0)
		{
			throw cet::exception("ArgumentOutOfRange") << "Buffer overrun detected! ContainerFragment::at was asked for a non-existent Fragment!";
		}
		FragmentPtr frag(new Fragment(fragSize(index) / sizeof(RawDataType) - detail::RawFragmentHeader::num_words()));
		memcpy(frag->headerAddress(), reinterpret_cast<uint8_t const *>(dataBegin()) + fragmentIndex(index), fragSize(index));
		return frag;
	}

	/**
	 * \brief Gets the size of the Fragment at the specified location in the ContainerFragment, in bytes
	 * \param index The Fragment index
	 * \return The size of the Fragment at the specified location in the ContainerFragment, in bytes
	 * \exception cet::exception if the index is out-of-range
	 */
	size_t fragSize(size_t index) const
	{
		if (index >= block_count() || block_count() == 0)
		{
			throw cet::exception("ArgumentOutOfRange") << "Buffer overrun detected! ContainerFragment::fragSize was asked for a non-existent Fragment!";
		}
		auto end = fragmentIndex(index + 1);
		if (index == 0) return end;
		return end - fragmentIndex(index);
	}

	/**
	 * \brief Alias to ContainerFragment::at()
	 * \param index The Fragment index to return
	 * \return Pointer to the specified Fragment in the ContainerFragment
	 * \exception cet::exception if the index is out-of-range
	 */
    FragmentPtr operator[](size_t index) const
	{
		return this->at(index);
	}

	/**
	 * \brief Get the offset of a Fragment within the ContainerFragment
	 * \param index The Fragment index
	 * \return The offset of the requested Fragment within the ContainerFragment
	 * \exception cet::exception if the index is out-of-range
	 */
	size_t fragmentIndex(size_t index) const
	{
		if (index > block_count())
		{
			throw cet::exception("ArgumentOutOfRange") << "Buffer overrun detected! ContainerFragment::fragmentIndex was asked for a non-existent Fragment!";
		}
		if (index == 0) { return 0; }

		auto index_ptr = get_index_();

		return index_ptr[index - 1];
	}

	/**
	 * \brief Returns the offset of the last Fragment in the ContainerFragment
	 * \return The offset of the last Fragment in the ContainerFragment
	 */
	size_t lastFragmentIndex() const
	{
		return fragmentIndex(block_count());
	}

protected:

	/**
	 * \brief Gets the ratio between the fundamental data storage type and the representation within the Fragment
	 * \return The ratio between the fundamental data storage type and the representation within the Fragment
	 */
	static constexpr size_t words_per_frag_word_()
	{
		return sizeof(Fragment::value_type) / sizeof(Metadata::data_t);
	}

	const size_t* create_index_() const
	{
		TLOG(TLVL_DEBUG, "ContainerFragment") << "Creating new index for ContainerFragment";
		auto tmp = new size_t[metadata()->block_count + 1];

		auto current = reinterpret_cast<uint8_t const*>(artdaq_Fragment_.dataBegin());
		size_t offset = 0;
		for (int ii = 0; ii < metadata()->block_count; ++ii)
		{
			auto this_size = reinterpret_cast<const detail::RawFragmentHeader*>(current)->word_count * sizeof(RawDataType);
			offset += this_size;
			tmp[ii] = offset;
			current += this_size;
		}
		tmp[metadata()->block_count] = CONTAINER_MAGIC;
		return tmp;
	}

	void reset_index_ptr_() const
	{
		TLOG(TLVL_DEBUG, "ContainerFragment") << "Request to reset index_ptr recieved. has_index=" << metadata()->has_index << ", Check word = " << std::hex
			<< *(reinterpret_cast<size_t const*>(artdaq_Fragment_.dataBeginBytes() + metadata()->index_offset) + metadata()->block_count);
		if (metadata()->has_index && *(reinterpret_cast<size_t const*>(artdaq_Fragment_.dataBeginBytes() + metadata()->index_offset) + metadata()->block_count) == CONTAINER_MAGIC)
		{
			TLOG(TLVL_DEBUG, "ContainerFragment") << "Setting index_ptr to found valid index";
			index_ptr_ = reinterpret_cast<size_t const*>(artdaq_Fragment_.dataBeginBytes() + metadata()->index_offset);
		}
		else
		{
			TLOG(TLVL_DEBUG, "ContainerFragment") << "Index invalid or not found, allocating new index";
			if (index_alloc_)
			{
				delete[] index_ptr_;
			}

			index_alloc_ = true;
			index_ptr_ = create_index_();
		}
	}

	const size_t* get_index_() const
	{
		if (index_ptr_ != nullptr) return index_ptr_;

		reset_index_ptr_();

		return index_ptr_;
	}

private:
	Fragment const& artdaq_Fragment_;

	mutable const size_t* index_ptr_;
	mutable bool index_alloc_;
	mutable const Metadata* metadata_;
	mutable bool metadata_alloc_;
};

#endif /* artdaq_core_Data_ContainerFragment_hh */
