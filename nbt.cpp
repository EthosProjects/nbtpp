#include "nbt.hpp"
namespace NBTPP {
    Reader::Reader(uint8_t* t_buffer, size_t t_size):  buffer(t_buffer), size(t_size) {}
    int8_t Reader::getByte() {
        if (offset >= size) throw std::out_of_range("Not enough data");
        return buffer[offset++];
    }
    int16_t Reader::getShort() {
        if (offset >= size-1) throw std::out_of_range("Not enough data");
        return ((int16_t) buffer[offset++] << 8) + (int16_t) buffer[offset++];
    }
    int32_t Reader::getInt() {
        if (offset >= size-3) throw std::out_of_range("Not enough data");
        return ((int32_t) buffer[offset++] << 24) + ((int32_t) buffer[offset++] << 16) + ((int32_t) buffer[offset++] << 8) + (int32_t) buffer[offset++];
    }
    int64_t Reader::getLong() {
        if (offset >= size-7) throw std::out_of_range("Not enough data");
        return ((int64_t) buffer[offset++] << 56) + ((int64_t) buffer[offset++] << 48) + ((int64_t) buffer[offset++] << 40) + ((int64_t) buffer[offset++] << 32) + 
                ((int64_t) buffer[offset++] << 24) + ((int64_t) buffer[offset++] << 16) + + ((int64_t) buffer[offset++] << 8) + (int64_t) buffer[offset++];
    }
    float Reader::getFloat() {
        return std::bit_cast<float>(getInt());
    }
    double Reader::getDouble() {
        return std::bit_cast<double>(getLong());
    }
    std::vector<int8_t> Reader::getByteArray() {
        int length = getInt();
        std::vector<int8_t> o_bytes;
        for (int i = 0; i < length; i++) {
            o_bytes.push_back(getByte());
        }
        return o_bytes;
    }
    std::string Reader::getString() {
        int length = getShort();
        offset+= length;
        return decodeUTF(length);
    }
    nlohmann::json Reader::getList() {
        nlohmann::json o_data;
        int type = getByte();
        int length = getInt();
        nlohmann::json values;
        bool unparseable = false;
        for (int i = 0; i < length; i++) {
            if (unparseable) throw std::invalid_argument("Failed to type at offset " + std::to_string(offset - 1));
            switch (type) {
                case TAGS::Byte : {
                    values.push_back(getByte());
                    break;
                }
                case TAGS::Short : {
                    values.push_back(getShort());
                    break;
                }
                case TAGS::Int : {
                    values.push_back(getInt());
                    break;
                }
                case TAGS::Long : {
                    values.push_back(getLong());
                    break;
                }
                case TAGS::Float : {
                    values.push_back(getFloat());
                    break;
                }
                case TAGS::Double : {
                    values.push_back(getDouble());
                    break;
                }
                case TAGS::ByteArray : {
                    values.push_back(getByteArray());
                    break;
                }
                case TAGS::String : {
                    values.push_back(getString());
                    break;
                }
                case TAGS::List : {
                    values.push_back(getList());
                    break;
                }
                case TAGS::Compound : {
                    values.push_back(getCompound());
                    break;
                }
                case TAGS::IntArray : {
                    values.push_back(getIntArray());
                    break;
                }
                case TAGS::LongArray : {
                    values.push_back(getLongArray());
                    break;
                }
                default: {
                    unparseable = true;	
                    break;
                }
            }
        }
        o_data["type"] = type;
        o_data["values"] = values;
        return o_data;
    }
    std::string Reader::decodeUTF(size_t t_length) {
        size_t t_offset = offset - t_length;
        char* o_char = new char[t_length];
        for (size_t i = 0; i < t_length; i++) {
            if ((buffer[i] & 0x80) == 0) {
                o_char[i] = buffer[t_offset + i] & 0x7F;
            } else if (i+1 < t_length &&
                        (buffer[t_offset + i]   & 0xE0) == 0xC0 &&
                        (buffer[t_offset + i+1] & 0xC0) == 0x80) {
                o_char[i] = (
                    ((buffer[t_offset + i]   & 0x1F) << 6) |
                    ( buffer[t_offset + i+1] & 0x3F));
            } else if (i+2 < t_length &&
                        (buffer[t_offset + i]   & 0xF0) == 0xE0 &&
                        (buffer[t_offset + i+1] & 0xC0) == 0x80 &&
                        (buffer[t_offset + i+2] & 0xC0) == 0x80) {
                o_char[i] = (
                    ((buffer[t_offset + i]   & 0x0F) << 12) |
                    ((buffer[t_offset + i+1] & 0x3F) <<  6) |
                    ( buffer[t_offset + i+2] & 0x3F));
            } else if (i+3 < t_length &&
                        (buffer[t_offset + i]   & 0xF8) == 0xF0 &&
                        (buffer[t_offset + i+1] & 0xC0) == 0x80 &&
                        (buffer[t_offset + i+2] & 0xC0) == 0x80 &&
                        (buffer[t_offset + i+3] & 0xC0) == 0x80) {
                o_char[i] = (
                    ((buffer[t_offset + i]   & 0x07) << 18) |
                    ((buffer[t_offset + i+1] & 0x3F) << 12) |
                    ((buffer[t_offset + i+2] & 0x3F) <<  6) |
                    ( buffer[t_offset + i+3] & 0x3F));
            }
        }
        std::string o_string(o_char, t_length);
        delete [] o_char;
        return o_string;
    }
    nlohmann::json Reader::getCompound() {
        nlohmann::json o_output;
        while (true) {
            int type = getByte();
            if (type == TAGS::End) {
                break;
            }
            std::string name = getString();
            bool unparseable = false;
            switch (type) {
                case TAGS::Byte : {
                    o_output[name] = getByte();
                    break;

                }
                case TAGS::Short : {
                    o_output[name] = getShort();
                    break;
                }
                case TAGS::Int : {
                    o_output[name] = getInt();
                    break;
                    
                }
                case TAGS::Long : {
                    o_output[name] = getLong();
                    break;
                }
                case TAGS::Float : {
                    o_output[name] = getFloat();
                    break;
                }
                case TAGS::Double : {
                    o_output[name] = getDouble();
                    break;
                }
                case TAGS::String : {
                    o_output[name] = getString();
                    break;
                }
                case TAGS::ByteArray : {
                    o_output[name] = getByteArray();
                    break;
                }
                case TAGS::List : {
                    o_output[name] = getList();
                    break;
                }
                case TAGS::Compound : {
                    o_output[name] = getCompound();
                    break;
                }
                case TAGS::IntArray : {
                    o_output[name] = getIntArray();
                    break;
                }
                case TAGS::LongArray : {
                    o_output[name] = getLongArray();
                    break;
                }
                default: {
                    unparseable = true;	
                    break;
                }
            }
            if (unparseable) throw std::invalid_argument("Failed to type at offset " + std::to_string(offset - 1));
        }
        return o_output;
    }
    std::vector<int32_t> Reader::getIntArray() {
        int length = getInt();
        std::vector<int32_t> o_bytes;
        for (size_t i = 0; i < length; i++) {
            o_bytes.push_back(getInt());
        }
        return o_bytes;
    }
    std::vector<int64_t> Reader::getLongArray() {
        int length = getInt();
        std::vector<int64_t> o_bytes;
        for (size_t i = 0; i < length; i++) {
            o_bytes.push_back(getLong());
        }
        return o_bytes;
    }

    nlohmann::json parseNBT(uint8_t* t_buffer, size_t t_size) {
        if (t_size < 1) throw std::exception("The buffer MUST HAVE LENGTH");
        Reader reader(t_buffer, t_size);
        if (reader.getByte() != TAGS::Compound) throw std::exception("Top tag MUST be a compound");
        std::string name = reader.getString();
        nlohmann::json o_data = reader.getCompound();
        return o_data;
    }
}
