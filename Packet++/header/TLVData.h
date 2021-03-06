#ifndef PACKETPP_TLV_DATA
#define PACKETPP_TLV_DATA

#include "Layer.h"
#include "IpAddress.h"
#include <string.h>

/// @file

/**
 * \namespace pcpp
 * \brief The main namespace for the PcapPlusPlus lib
 */
namespace pcpp
{
	/**
	 * @class TLVRecord
	 * A wrapper class for a Type-Length-Value (TLV) record. This class does not create or modify TLV records, but rather
	 * serves as a wrapper and provides useful methods for retrieving data from them. This class has several abstract methods
	 * that should be implemented in derived classes. These methods are for record length value calculation (the 'L' in TLV)
	 * which is implemented differently in different protocols
	 */
	class TLVRecord
	{
	protected:

		/** A struct representing the TLV construct */
		struct TLVRawData
		{
			/** Record type */
			uint8_t recordType;
			/** Record length in bytes */
			uint8_t recordLen;
			/** Record value (variable size) */
			uint8_t recordValue[];
		};

		TLVRawData* m_Data;

	public:

		/**
		 * A c'tor for this class that gets a pointer to the TLV record raw data (byte array)
		 * @param[in] recordRawData A pointer to the TLV record raw data
		 */
		TLVRecord(uint8_t* recordRawData)
		{
			if (recordRawData == NULL)
				m_Data = NULL;
			else
				m_Data = (TLVRawData*)recordRawData;
		}

		/**
		 * A copy c'tor for this class. This copy c'tor doesn't copy the TLV data, but only the pointer to it,
		 * which means that after calling it both the old and the new instance will point to the same TLV raw data
		 * @param[in] other The TLVRecord instance to copy from
		 */
		TLVRecord(const TLVRecord& other)
		{
			m_Data = other.m_Data;
		}

		/**
		 * A d'tor for this class, currently does nothing
		 */
		virtual ~TLVRecord() { }

		/**
		 * Overload of the assignment operator. This operator doesn't copy the TLV data, but rather copies the pointer to it,
		 * which means that after calling it both the old and the new instance will point to the same TLV raw data
		 * @param[in] other The TLVRecord instance to assign
		 */
		TLVRecord& operator=(const TLVRecord& other)
		{
			m_Data = other.m_Data;
			return *this;
		}

		/**
		 * @return The type field of the record (the 'T' in __Type__-Length-Value)
		 */
		uint8_t getType() { return m_Data->recordType; }

		/**
		 * @return A pointer to the value of the record as byte array (the 'V' in Type-Length- __Value__)
		 */
		uint8_t* getValue() { return m_Data->recordValue; }

		/**
		 * @return True if the TLV record raw data is NULL which, false otherwise
		 */
		bool isNull() { return (m_Data == NULL); }

		/**
		 * @return A pointer to the TLV record raw data byte stream
		 */
		uint8_t* getRecordBasePtr() { return (uint8_t*)m_Data; }

		/**
		 * Free the memory of the TLV record raw data
		 */
		void purgeRecordData() { if (!isNull()) delete m_Data; }

		/**
		 * A templated method to retrieve the record data as a certain type T. For example, if record data is 4B long
		 * (integer) then this method should be used as getValueAs<int>() and it will return the record data as an integer.<BR>
		 * Notice this return value is a copy of the data, not a pointer to the actual data
		 * @return The record data as type T
		 */
		template<typename T>
		T getValueAs()
		{
			if (getDataSize() < sizeof(T))
				return 0;

			T result;
			memcpy(&result, m_Data->recordValue, sizeof(T));
			return result;
		}

		/**
		 * @return The total size of the TLV record (in bytes)
		 */
		virtual size_t getTotalSize() const = 0;

		/**
		 * @return The size of the record value (meaning the size of the 'V' part in TLV)
		 */
		virtual size_t getDataSize() = 0;

	};


	/**
	 * @class TLVRecordReader
	 * A class for reading TLV records data out of a byte stream. This class contains helper methods for retrieving and
	 * counting TLV records. This is a template class that expects template argument class derived from TLVRecord.
	 */
	template<typename TLVRecordType>
	class TLVRecordReader
	{
	private:
		size_t m_RecordCount;

	public:

		/**
		 * A default c'tor for this class
		 */
		TLVRecordReader() { m_RecordCount = (size_t)-1; }

		/**
		 * A d'tor for this class which currently does nothing
		 */
		virtual ~TLVRecordReader() { }

		/**
		 * Get the first TLV record out of a byte stream
		 * @param[in] tlvDataBasePtr A pointer to the TLV data byte stream
		 * @param[in] tlvDataLen The TLV data byte stream length
		 * @return An instance of type TLVRecordType that contains the first TLV record. If tlvDataBasePtr is NULL or
		 * tlvDataLen is zero the returned TLVRecordType instance will be logically NULL, meaning TLVRecordType.isNull() will
		 * return true
		 */
		TLVRecordType getFirstTLVRecord(uint8_t* tlvDataBasePtr, size_t tlvDataLen)
		{
			// check if there are records at all
			if (tlvDataLen == 0)
				return TLVRecordType(NULL);

			return TLVRecordType(tlvDataBasePtr);
		}

		/**
		 * Get a TLV record that follows a given TLV record in a byte stream
		 * @param[in] record A given TLV record
		 * @param[in] tlvDataBasePtr A pointer to the TLV data byte stream
		 * @param[in] tlvDataLen The TLV data byte stream length
		 * @return An instance of type TLVRecordType that wraps the record following the record given as input. If the
		 * input record.isNull() is true or if the next record is out of bounds of the byte stream, a logical NULL instance
		 * of TLVRecordType will be returned, meaning TLVRecordType.isNull() will return true
		 */
		TLVRecordType getNextTLVRecord(TLVRecordType& record, uint8_t* tlvDataBasePtr, size_t tlvDataLen)
		{
			if (record.isNull())
				return TLVRecordType(NULL);

			// record pointer is out-bounds of the TLV records memory
			if ((record.getRecordBasePtr() - tlvDataBasePtr) < 0)
				return TLVRecordType(NULL);

			// record pointer is out-bounds of the TLV records memory
			if (record.getRecordBasePtr() - tlvDataBasePtr + (int)record.getTotalSize()  >= (int)tlvDataLen)
				return TLVRecordType(NULL);

			return TLVRecordType(record.getRecordBasePtr() + record.getTotalSize());
		}

		/**
		 * Search for the first TLV record that corresponds to a given record type (the 'T' in __Type__-Length-Value)
		 * @param[in] recordType The record type to search for
		 * @param[in] tlvDataBasePtr A pointer to the TLV data byte stream
		 * @param[in] tlvDataLen The TLV data byte stream length
		 * @return An instance of type TLVRecordType that contains the result record. If record was not found a logical
		 * NULL instance of TLVRecordType will be returned, meaning TLVRecordType.isNull() will return true
		 */
		TLVRecordType getTLVRecord(uint8_t recordType, uint8_t* tlvDataBasePtr, size_t tlvDataLen)
		{
			TLVRecordType curRec = getFirstTLVRecord(tlvDataBasePtr, tlvDataLen);
			while (!curRec.isNull())
			{
				if (curRec.getType() == recordType)
					return curRec;

				curRec = getNextTLVRecord(curRec, tlvDataBasePtr, tlvDataLen);
			}

			return TLVRecordType(NULL);
		}

		/**
		 * Get the TLV record count in a given TLV data byte stream. For efficiency purposes the count is being cached
		 * so only the first call to this method will go over all the TLV records, while all consequent calls will return
		 * the cached number. This implies that if there is a change in the number of records, it's the user's responsibility
		 * to call changeTLVRecordCount() with the record count change
		 * @param[in] tlvDataBasePtr A pointer to the TLV data byte stream
		 * @param[in] tlvDataLen The TLV data byte stream length
		 * @return The TLV record count
		 */
		size_t getTLVRecordCount(uint8_t* tlvDataBasePtr, size_t tlvDataLen)
		{
			if (m_RecordCount != (size_t)-1)
				return m_RecordCount;

			m_RecordCount = 0;
			TLVRecordType curRec = getFirstTLVRecord(tlvDataBasePtr, tlvDataLen);
			while (!curRec.isNull())
			{
				m_RecordCount++;
				curRec = getNextTLVRecord(curRec, tlvDataBasePtr, tlvDataLen);
			}

			return m_RecordCount;
		}

		/**
		 * As described in getTLVRecordCount(), the TLV record count is being cached for efficiency purposes. So if the
		 * number of TLV records change, it's the user's responsibility to call this method with the number of TLV records
		 * being added or removed. If records were added the change should be a positive number, or a negative number
		 * if records were removed
		 * @param[in] changedBy Number of records that were added or removed
		 */
		void changeTLVRecordCount(int changedBy) { if (m_RecordCount != (size_t)-1) m_RecordCount += changedBy; }
	};


	/**
	 * @class TLVRecordBuilder
	 * A base class for building Type-Length-Value (TLV) records. This builder receives the record parameters in its c'tor,
	 * builds the record raw buffer and provides a method to build a TLVRecord object out of it. Please notice this is
	 * a base class that lacks the capability of actually building TLVRecord objects and also cannot be instantiated. The
	 * reason for that is that different protocols build TLV records in different ways, so these missing capabilities will
	 * be implemented by the derived classes which are specific to each protocol. This class only provides the common
	 * infrastructure that will be used by them
	 */
	class TLVRecordBuilder
	{
	protected:

		// unimplemented default c'tor
		TLVRecordBuilder();

		TLVRecordBuilder(uint8_t recType, const uint8_t* recValue, uint8_t recValueLen);

		TLVRecordBuilder(uint8_t recType, uint8_t recValue);

		TLVRecordBuilder(uint8_t recType, uint16_t recValue);

		TLVRecordBuilder(uint8_t recType, uint32_t recValue);

		TLVRecordBuilder(uint8_t recType, const IPv4Address& recValue);

		TLVRecordBuilder(uint8_t recType, const std::string& recValue);

		TLVRecordBuilder(const TLVRecordBuilder& other);

		virtual ~TLVRecordBuilder();

		void init(uint8_t recType, const uint8_t* recValue, uint8_t recValueLen);

		uint8_t* m_RecValue;
		uint8_t m_RecValueLen;
		uint8_t m_RecType;
	};
}
#endif // PACKETPP_TLV_DATA
