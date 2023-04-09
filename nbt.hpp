#include <stdint.h>
#include <nlohmann/json.hpp>
#include <exception>
namespace NBTPP {
	namespace TAGS {
		enum TAGS {
			End = 0,
			Byte = 1,
			Short = 2,
			Int = 3,
			Long = 4,
			Float = 5,
			Double = 6,
			ByteArray = 7,
			String = 8,
			List = 9,
			Compound = 10,
			IntArray = 11,
			LongArray = 12
		};
	}
	struct Reader {
		int offset = 0;
		size_t size = 0;
		uint8_t* buffer;
		Reader(uint8_t* t_buffer, size_t t_size);
		int8_t getByte();
		int16_t getShort();
		int32_t getInt();
		int64_t getLong();
		float getFloat();
		double getDouble();
		std::vector<int8_t> getByteArray();
		std::string getString();
		nlohmann::json getList();
		std::string decodeUTF(size_t t_length);
		nlohmann::json getCompound();
		std::vector<int32_t> getIntArray();
		std::vector<int64_t> getLongArray();
	};
	nlohmann::json parseNBT(uint8_t* t_buffer, size_t t_size);
}
