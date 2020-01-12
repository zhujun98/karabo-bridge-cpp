/*
    Karabo bridge client.

    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_KB_CLIENT_HPP
#define KARABO_BRIDGE_KB_CLIENT_HPP

#include <zmq.hpp>

#include <string>
#include <vector>
#include <stack>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <limits>
#include <type_traits>

#include "kb_data.hpp"


namespace karabo_bridge {

class ZmqTimeoutError : public std::runtime_error {
public:
    ZmqTimeoutError() : std::runtime_error("") {}
};

/*
 * Convert a vector to a formatted string
 */
template <typename T>
std::string vectorToString(const std::vector<T>& vec) {
    if (vec.empty()) return "";

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
 * Convert the python type to the corresponding C++ type
 */
inline void toCppTypeString(std::string& dtype) {
    if (dtype.find("int") != std::string::npos)
        dtype.append("_t");
    else if (dtype == "float32")
        dtype = "float";
    else if (dtype == "float64")
        dtype = "double";
}

/*
 * Karabo-bridge Client class.
 */
class Client {
    zmq::context_t ctx_;
    zmq::socket_t socket_;

    // Set to true if the client has sent request to the server to ask
    // for data.
    bool recv_ready_ = false;

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
            auto flag = socket_.recv(&msg);
            if (!flag) throw ZmqTimeoutError();

            mpmsg.emplace_back(std::move(msg));
            std::size_t more_size = sizeof(int64_t);
            socket_.getsockopt(ZMQ_RCVMORE, &more, &more_size);
            if (more == 0) break;
        }
        return mpmsg;
    }

    /*
     * Parse a single message packed by msgpack using "visitor".
     */
    static std::string parseMsg(const zmq::message_t& msg) {
        /*
         * Visitor used to unfold the hierarchy of an unknown data structure,
         */
        struct visitor {
            std::string& m_s;
            bool m_ref;

            explicit visitor(std::string& s):m_s(s), m_ref(false) {}
            ~visitor() { m_s += "\n"; }

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
                std::cout << "insufficient bytes" << std::endl;
            }

            // These two functions are required by parser.
            void set_referenced(bool ref) { m_ref = ref; }
            bool referenced() const { return m_ref; }

        private:
            std::stack<int> tracker_;
            uint16_t level_ = 0;
            bool is_key_ = false;
        };

        std::string data_str;
        visitor vst(data_str);
    #ifdef DEBUG
        assert(msgpack::parse(static_cast<const char*>(msg.data()), msg.size(), visitor));
    #else
        msgpack::parse(static_cast<const char*>(msg.data()), msg.size(), vst);
    #endif
        return data_str;
    }

    /*
     * Parse a multipart message packed by msgpack using "visitor".
     */
    static std::string parseMultipartMsg(const MultipartMsg& mpmsg, bool boundary=true) {
        std::string output;
        std::string separator("\n----------new message----------\n");
        for (auto& msg : mpmsg) {
            if (boundary) output.append(separator);
            output.append(parseMsg(msg));
        }
        return output;
    }

    /*
     * Add formatted output to a stringstream.
     */
    template <typename T>
    void prettyStream(const std::pair<std::string, T>& v, std::stringstream& ss) {
        ss << v.first
           << ", " << v.second.containerType()
           << ", " << vectorToString(v.second.shape())
           << ", " << v.second.dtype();

        if (v.second.containerType() == "map" || v.second.containerType() == "MSGPACK_OBJECT_EXT")
            ss << " (Check...unexpected data type!)";
        ss << "\n";
    }

public:
    /*
     * Constructor.
     *
     * @param timeout: connection timeout in second. "-1." (default) for infinite.
     */
    explicit Client(double timeout=-1.): ctx_(1), socket_(ctx_, ZMQ_REQ) {
      socket_.setsockopt(ZMQ_RCVTIMEO, timeout < 0 ? -1 : static_cast<int>(1000 * timeout));
      socket_.setsockopt(ZMQ_LINGER, 0);
    }

    // The destructor of zmq::context_t calls 'zmq_ctx_destroy'.
    // The destructor of zmq::socket_t calls 'zmq_close'.
    ~Client() = default;

    // The copy and copy assignment constructor are implicitly deleted since
    // those of zmq::context_t and zmq::socket_t are deleted.
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    void connect(const std::string& endpoint) {
        std::cout << "Connecting to server: " << endpoint << std::endl;
        socket_.connect(endpoint);
    }

    /*
     * Request and return the next data from the server.
     *
     * Exceptions:
     * std::runtime_error if unexpected message number or unknown "content" is found
     */
    std::map<std::string, KbData> next() {
        std::map<std::string, KbData> data_pkg;

        if (!recv_ready_) {
            sendRequest();
            recv_ready_ = true;
        }

        MultipartMsg mpmsg;
        try {
            mpmsg = receiveMultipartMsg();
            recv_ready_ = false;
        } catch (const ZmqTimeoutError&) {
            return data_pkg;
        }

        if (mpmsg.empty()) return data_pkg;

        if (mpmsg.size() % 2)
            throw std::runtime_error(
                "The multipart message is expected to contain (header, data) pairs!");

        KbData kbdt;

        std::string source;
        bool is_initialized = false;
        auto it = mpmsg.begin();
        while(it != mpmsg.end()) {
            // the header must contain "source" and "content"
            msgpack::object_handle oh_header;
            msgpack::unpack(oh_header, static_cast<const char*>(it->data()), it->size());
            auto header_unpacked = oh_header.get().as<ObjectMap>();

            auto content = header_unpacked.at("content").as<std::string>();

            // the next message is the content (data)
            if (content == "msgpack") {
                if (!is_initialized)
                    is_initialized = true;
                else {
                    data_pkg.insert(std::make_pair(source, std::move(kbdt)));
                    // TODO: the following 'swap" seems to be redundant
                    KbData empty_data;
                    kbdt.swap(empty_data);
                }

                kbdt.appendMsg(std::move(*it));
                std::advance(it, 1);

                msgpack::object_handle oh_data;
                msgpack::unpack(oh_data, static_cast<const char*>(it->data()), it->size());
                kbdt.metadata = header_unpacked.at("metadata").as<ObjectMap>();

                auto data_unpacked = oh_data.get().as<ObjectMap>();
                for (auto& v : data_unpacked) kbdt.insert(v); // shallow copy

                kbdt.appendHandle(std::move(oh_header));
                kbdt.appendHandle(std::move(oh_data));

            } else if ((content == "array" || content == "ImageData")) {
                kbdt.appendMsg(std::move(*it));
                std::advance(it, 1);

                auto tmp = header_unpacked.at("shape").as<std::vector<unsigned int>>();
                std::vector<std::size_t> shape(tmp.begin(), tmp.end());
                auto dtype = header_unpacked.at("dtype").as<std::string>();
                toCppTypeString(dtype);

                kbdt.array.insert(std::make_pair(header_unpacked.at("path").as<std::string>(),
                                                 NDArray(it->data(), shape, dtype)));
            } else {
                throw std::runtime_error("Unknown data content: " + content);
            }

            source = header_unpacked.at("source").as<std::string>();

            kbdt.appendMsg(std::move(*it));
            std::advance(it, 1);
        }

        data_pkg.insert(std::make_pair(source, std::move(kbdt)));
        KbData empty_data;
        kbdt.swap(empty_data);

        return data_pkg;
    }

    /*
     * Parse the next multipart message.
     *
     * Note:: this member function consumes data!!!
     */
    std::string showMsg() {
        sendRequest();
        auto mpmsg = receiveMultipartMsg();
        return parseMultipartMsg(mpmsg);
    }

    /*
     * Parse the data structure of the received kb_data.
     *
     * Note:: this member function consumes data!!!
     */
    std::string showNext() {
        auto data_pkg = next();

        std::stringstream ss;
        for (auto& data : data_pkg) {
            ss << "source: " << data.first << "\n";
            ss << "Total bytes received: " << data.second.bytesReceived() << "\n\n";

            ss << "path, container, container shape, type\n";

            ss << "\nmetadata\n" << std::string(8, '-') << "\n";
            for (auto& v : data.second.metadata) prettyStream<MsgpackObject>(v, ss);

            ss << "\ndata\n" << std::string(4, '-') << "\n";
            for (auto& v : data.second) prettyStream<MsgpackObject>(v, ss);

            ss << "\narray\n" << std::string(5, '-') << "\n";
            for (auto& v : data.second.array) prettyStream<NDArray>(v, ss);

            ss << "\n";
        }

        return ss.str();
    }
};

} // karabo_bridge

#endif //KARABO_BRIDGE_KB_CLIENT_HPP
