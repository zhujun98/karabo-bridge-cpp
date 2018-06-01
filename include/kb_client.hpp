/*
    Karabo bridge client.

    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_CPP_KB_CLIENT_HPP
#define KARABO_BRIDGE_CPP_KB_CLIENT_HPP

#include <zmq.hpp>
#include <msgpack.hpp>

#include <string>
#include <stack>
#include <array>
#include <deque>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <limits>
#include <type_traits>


namespace karabo_bridge {

/*
  typedef enum {
      MSGPACK_OBJECT_NIL                  = 0x00,
      MSGPACK_OBJECT_BOOLEAN              = 0x01,
      MSGPACK_OBJECT_POSITIVE_INTEGER     = 0x02,
      MSGPACK_OBJECT_NEGATIVE_INTEGER     = 0x03,
      MSGPACK_OBJECT_FLOAT32              = 0x0a,
      MSGPACK_OBJECT_FLOAT64              = 0x04,
      MSGPACK_OBJECT_FLOAT                = 0x04,
      MSGPACK_OBJECT_STR                  = 0x05,
      MSGPACK_OBJECT_ARRAY                = 0x06,
      MSGPACK_OBJECT_MAP                  = 0x07,
      MSGPACK_OBJECT_BIN                  = 0x08,
      MSGPACK_OBJECT_EXT                  = 0x09
  } msgpack_object_type;

  NIL indicates either msgpack::NIL object or the data type in an empty
  msgpack::ARRAY.
*/
const std::vector<std::string> object_type = {
    "NIL", "bool", "uint64_t", "int64_t", "double", "string",
    "msgpack::ARRAY", "msgpack::MAP", "msgpack::BIN", "msgpack::EXT", "float"
};

/*
 * A container hold a msgpack::object for deferred unpack.
 */
class Object {

    msgpack::object value_;

public:
    Object() = default;  // must be default constructable

    explicit Object(const msgpack::object& value): value_(value) {}

    ~Object() = default;

    Object(const Object&) = default;

    Object& operator=(const Object&) = default;

    Object(Object&&) noexcept = default;

    Object& operator=(Object&&) noexcept = default;

    /*
     * Cast the held msgpack::object to a given type.
     *
     * Exceptions:
     * std::bad_cast if the cast fails.
     */
    template<typename T>
    T as() {
        return value_.as<T>();
    }

    msgpack::object get() const { return value_; }

    int dtype() const { return value_.type; }
};

/*
 * A container held a pointer to a char array converted from a byte stream
 * and other useful information.
 */
class Array {
    zmq::message_t msg_; // cannot be copied
    std::vector<unsigned int> shape_; // shape of the array
    std::string dtype_; // data type

    std::size_t size() const {
        auto max_size = std::numeric_limits<unsigned long long>::max();

        std::size_t size = 1;
        for (auto v : shape_) {
            if (max_size/size < v)
                throw std::overflow_error("Unmanageable array size!");
            size *= v;
        }

        return size;
    }
public:
    Array() = default;

    // shape and dtype should be moved into the constructor
    Array(zmq::message_t msg, std::vector<unsigned int> shape, std::string dtype):
        msg_(std::move(msg)),
        shape_(std::move(shape)),
        dtype_(std::move(dtype)) {}

    Array(const Array& rhs) : shape_(rhs.shape_), dtype_(rhs.dtype_) {
        msg_.copy(&rhs.msg_);
    };

    Array& operator=(const Array& rhs) {
        msg_.copy(&rhs.msg_);
        shape_ = rhs.shape_;
        dtype_ = rhs.dtype_;

        return *this;
    };

    Array(Array&&) = default;
    Array& operator=(Array&&) = default;

    /*
     * Cast the held const char* array to std::vector<T>.
     *
     * Note: users are responsible to give the correct data type
     *       otherwise it leads to undefined behavior.
     *
     * Exceptions:
     * std::bad_cast if the cast fails or the types do not match
     */
    template<typename T>
    std::vector<T> as() {
        // type check
        if ((dtype_ == "uint64" && ! std::is_same<T, uint64_t>::value) ||
            (dtype_ == "uint32" && ! std::is_same<T, uint32_t>::value) ||
            (dtype_ == "uint16" && ! std::is_same<T, uint16_t>::value) ||
            (dtype_ == "uint8" && ! std::is_same<T, uint8_t>::value) ||
            (dtype_ == "int64" && ! std::is_same<T, int64_t>::value) ||
            (dtype_ == "int32" && ! std::is_same<T, int32_t>::value) ||
            (dtype_ == "int16" && ! std::is_same<T, int16_t>::value) ||
            (dtype_ == "int8" && ! std::is_same<T, int8_t>::value) ||
            (dtype_ == "float" && ! std::is_same<T, float>::value) ||
            (dtype_ == "double" && ! std::is_same<T, double>::value)) {
            std::cerr << "Template type and data type '" + dtype_ + "' do not match!\n";
            throw std::bad_cast();
        }

        auto ptr = reinterpret_cast<const T*>(msg_.data());
        // TODO: avoid the copy
        return std::vector<T>(ptr, ptr + size());
    }

    std::vector<unsigned int> shape() const { return shape_; }

    std::string dtype() const { return dtype_; }
};

} // karabo_bridge


namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor{

/*
 * template specialization for karabo_bridge::object
 */
template<>
struct as<karabo_bridge::Object> {
    karabo_bridge::Object operator()(msgpack::object const& o) const {
        return karabo_bridge::Object(o.as<msgpack::object>());
    }
};

} // adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // msgpack


namespace karabo_bridge {

/*
 * Visitor used to unfold the hierarchy of an unknown data structure,
 */
struct karabo_visitor {
    std::string& m_s;
    bool m_ref;

    explicit karabo_visitor(std::string& s):m_s(s), m_ref(false) {}
    ~karabo_visitor() { m_s += "\n"; }

    bool visit_nil() {
        m_s += "null";
        return true;
    }
    bool visit_boolean(bool v) {
        if (v) m_s += "true";
        else m_s += "false";
        return true;
    }
    bool visit_positive_integer(uint64_t v) {
        std::stringstream ss;
        ss << v;
        m_s += ss.str();
        return true;
    }
    bool visit_negative_integer(int64_t v) {
        std::stringstream ss;
        ss << v;
        m_s += ss.str();
        return true;
    }
    bool visit_float32(float v) {
        std::stringstream ss;
        ss << v;
        m_s += ss.str();
        return true;
    }
    bool visit_float64(double v) {
        std::stringstream ss;
        ss << v;
        m_s += ss.str();
        return true;
    }
    bool visit_str(const char* v, uint32_t size) {
        m_s += '"' + std::string(v, size) + '"';
        return true;
    }
    bool visit_bin(const char* v, uint32_t size) {
        if (is_key_)
            m_s += std::string(v, size);
        else m_s += "(bin)";
        return true;
    }
    bool visit_ext(const char* /*v*/, uint32_t /*size*/) {
        return true;
    }
    bool start_array_item() {
        return true;
    }
    bool start_array(uint32_t /*size*/) {
        m_s += "[";
        return true;
    }
    bool end_array_item() {
        m_s += ",";
        return true;
    }
    bool end_array() {
        m_s.erase(m_s.size() - 1, 1); // remove the last ','
        m_s += "]";
        return true;
    }
    bool start_map(uint32_t /*num_kv_pairs*/) {
        tracker_.push(level_++);
        return true;
    }
    bool start_map_key() {
        is_key_ = true;
        m_s += "\n";

        for (int i=0; i< tracker_.top(); ++i) m_s += "    ";
        return true;
    }
    bool end_map_key() {
        m_s += ": ";
        is_key_ = false;
        return true;
    }
    bool start_map_value() {
        return true;
    }
    bool end_map_value() {
        m_s += ",";
        return true;
    }
    bool end_map() {
        m_s.erase(m_s.size() - 1, 1); // remove the last ','
        tracker_.pop();
        --level_;
        return true;
    }
    void parse_error(size_t /*parsed_offset*/, size_t /*error_offset*/) {
        std::cerr << "parse error"<<std::endl;
    }
    void insufficient_bytes(size_t /*parsed_offset*/, size_t /*error_offset*/) {
        std::cout << "insufficient bytes"<<std::endl;
    }

    // These two functions are required by parser.
    void set_referenced(bool ref) { m_ref = ref; }
    bool referenced() const { return m_ref; }

private:
    std::stack<int> tracker_;
    uint16_t level_ = 0;
    bool is_key_ = false;
};


/*
 * Data structure presented to the user.
 *
 * There are two different types of array: one is msgpack::ARRAY which is
 * encapsulated by Object and another is char array which is converted from
 * a byte stream and encapsulated by class Array.
 */
struct kb_data {
    std::map<std::string, Object> msgpack_data;
    std::map<std::string, Array> array;

    Object& operator[](const std::string& key) {
        return msgpack_data.at(key);
    }

    std::size_t size() { return size_; }
    void setSize(std::size_t s) { size_ = s; }

private:
    // size_ is the sum of the sizes of the received zmq messages.
    std::size_t size_;
};

using MsgObjectMap = std::map<std::string, msgpack::object>;
using MultipartMsg = std::deque<zmq::message_t>;

/*
 * Parse a single message packed by msgpack using "visitor".
 */
std::string parseMsg(const zmq::message_t& msg) {
    std::string data_str;
    karabo_visitor visitor(data_str);
#ifdef DEBUG
    assert(msgpack::parse(static_cast<const char*>(msg.data()), msg.size(), visitor));
#else
    msgpack::parse(static_cast<const char*>(msg.data()), msg.size(), visitor);
#endif
    return data_str;
}

/*
 * Parse a multipart message packed by msgpack using "visitor".
 */
std::string parseMultipartMsg(const MultipartMsg& mpmsg, bool boundary=true) {
    std::string output;
    std::string separator("\n----------new message----------\n");
    for (auto& msg : mpmsg) {
        if (boundary) output.append(separator);
        output.append(parseMsg(msg));
    }
    return output;
}

/*
 * Unpack a dictionary like message.
 *
 * The msg must be moved into the function and destroyed here.
 *
 * Exceptions:
 * std::bad_cast if the msg cannot be converted to std::map<std::string, msgpack::object>
 */
MsgObjectMap unpack_msg(zmq::message_t&& msg) {
    msgpack::object_handle oh;
    msgpack::unpack(oh, static_cast<const char*>(msg.data()), msg.size());
    return oh.get().as<MsgObjectMap>();
}

/*
 * Convert a vector to a formatted string
 */
template <typename T>
std::string vector2string(const std::vector<T>& vec) {
    std::stringstream ss;
    ss << "[";
    for (std::size_t i=0; i<vec.size(); ++i) {
        ss << vec[i];
        if (i != vec.size() - 1) ss << ", ";
    }
    ss << "]";
    return ss.str();
}

/*
 * Karabo-bridge Client class.
 */
class Client {
    zmq::context_t ctx_;
    zmq::socket_t socket_;

    /*
     * Send a "next" request to server.
     */
    void sendRequest() {
        zmq::message_t request(4);
        memcpy(request.data(), "next", request.size());
        socket_.send(request);
    }

    /*
     * Receive a multipart message from the server.
     */
    MultipartMsg receiveMultipartMsg() {
        int64_t more;  // multipart checker
        MultipartMsg mpmsg;
        while (true) {
            zmq::message_t msg;
            socket_.recv(&msg);
            mpmsg.emplace_back(std::move(msg));
            std::size_t more_size = sizeof(int64_t);
            socket_.getsockopt(ZMQ_RCVMORE, &more, &more_size);
            if (more == 0) break;
        }
        return mpmsg;
    }

public:
    Client(): ctx_(1), socket_(ctx_, ZMQ_REQ) {}

    void connect(const std::string& endpoint) {
        std::cout << "Connecting to server: " << endpoint << std::endl;
        socket_.connect(endpoint.c_str());
    }

    /*
     * Request and return the next data from the server.
     *
     * Exceptions:
     * std::runtime_error if unknown "content" is found
     */
    std::map<std::string, kb_data> next() {
        std::map<std::string, kb_data> data_pkg;

        sendRequest();
        MultipartMsg mpmsg = receiveMultipartMsg();
        if (mpmsg.empty()) return data_pkg;
        if (mpmsg.size() % 2)
            throw std::runtime_error("The multipart message is expected to "
                                     "contain even number of messages!");

        kb_data kbdt;
        std::string source;
        std::size_t byte_recv = 0;
        bool is_initialized = false;
        auto it = mpmsg.begin();
        while(it != mpmsg.end()) {
            // the header must contain "source" and "content"
            auto header_unpacked = unpack_msg(std::move(*it));
            source = header_unpacked.at("source").as<std::string>();
            auto content = header_unpacked.at("content").as<std::string>();

            // the next message is the content (data)
            std::advance(it, 1);
            if (content == "msgpack") {
                if (!is_initialized)
                    is_initialized = true;
                else {
                    kbdt.setSize(byte_recv);
                    data_pkg.insert(std::make_pair(std::move(source), std::move(kbdt)));
                    kbdt = {};
                    byte_recv = 0;
                }
                byte_recv += it->size();
                auto data_unpacked = unpack_msg(std::move(*it));
                for (auto &dt : data_unpacked)
                    kbdt.msgpack_data.insert(std::make_pair(dt.first, dt.second.as<Object>()));
            } else if ((content == "array" || content == "ImageData")) {
                byte_recv += it->size();
                auto shape = header_unpacked.at("shape").as<std::vector<unsigned int>>();
                auto dtype = header_unpacked.at("dtype").as<std::string>();

                kbdt.array.insert(std::make_pair(
                    header_unpacked.at("path").as<std::string>(),
                    Array(std::move(*it), std::move(shape), std::move(dtype))));
            } else {
                throw std::runtime_error("Unknown data content: " + content);
            }

            std::advance(it, 1);
        }

        kbdt.setSize(byte_recv);
        data_pkg.insert(std::make_pair(std::move(source), std::move(kbdt)));

        return data_pkg;
    }

    /*
     * Parse the next multipart message.
     *
     * Note:: this function consumes data!!!
     */
    std::string showMsg() {
        sendRequest();
        auto mpmsg = receiveMultipartMsg();
        return parseMultipartMsg(mpmsg);
    }

    /*
     * Parse the data structure of the received kb_data.
     *
     * Note:: this function consumes data!!!
     */
    std::string showNext() {
        auto data_pkg = next();

        std::stringstream ss;
        for (auto& data : data_pkg) {
            ss << "source: " << data.first << "\n";
            ss << "Total bytes received: " << data.second.size()
               << " (header messages are excluded) \n\n";

            ss << "path, type, container data type, container shape\n";
            for (auto& v : data.second.msgpack_data) {
                int idx = v.second.dtype();

                ss << v.first << ", ";

                if (idx == 0) { // msgpack::NIL
                    ss << object_type[idx] << " (Check...unexpected data type!)\n";
                } else if (idx == 6 ) { // msgpack::ARRAY
                    int size = v.second.get().via.array.size;
                    ss << object_type[idx] << ", ";
                    if (!size) {
                        ss << object_type[0] << ", [" << size << "]\n";
                    } else {
                        if (v.first == "image.passport") {
//                            ss << v.second.get() << "\n";
                            // TODO: check the array of string
                            ss << "string" << ", [" << size << "]\n";
                        } else {
                            int idxx = v.second.get().via.array.ptr[1].type;
                            ss << object_type[idxx] << ", [" << size << "]\n";
                        }
                    }

                } else if (idx == 7) { // msgpack::MAP
                    ss << object_type[idx] << " (Check...unexpected data type!)\n";

                } else if (idx == 8 ) { // msgpack::BIN
                    int size = v.second.get().via.bin.size;
                    ss << object_type[idx] << ", " << "int8_t" << ", [" << size << "]\n";

                } else if (idx == 9) { // msgpack::EXT
                    ss << object_type[idx] << " (Check...unexpected data type!)\n";

                } else {
                    ss << object_type[v.second.dtype()] << "\n";
                }
            }

            for (auto &v : data.second.array) {
                ss << v.first << ": " << "Array" << ", " << v.second.dtype()
                   << ", " << vector2string(v.second.shape()) << "\n";
            }

            ss << "\n";
        }

        return ss.str();
    }
};

} // karabo_bridge

#endif //KARABO_BRIDGE_CPP_KB_CLIENT_HPP
