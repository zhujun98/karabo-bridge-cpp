/*
    Karabo bridge client.

    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_KB_DATA_HPP
#define KARABO_BRIDGE_KB_DATA_HPP

#include <zmq.hpp>
#include <msgpack.hpp>

#include <string>
#include <vector>
#include <map>
#include <array>
#include <deque>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <limits>
#include <type_traits>


namespace karabo_bridge {

using MultipartMsg = std::deque<zmq::message_t>;

// Define exceptions to ease debugging

class CastError : public std::exception {
    std::string msg_;
public:
    explicit CastError(const std::string& msg) : msg_(msg) {}
    virtual const char* what() const throw() {
        return msg_.c_str() ;
    }
};

class TypeMismatchErrorNDArray : public CastError {
public:
    explicit TypeMismatchErrorNDArray(const std::string& msg)
        : CastError(msg)
    {}
};

class CastErrorMsgpackObject : public CastError {
public:
    explicit CastErrorMsgpackObject(const std::string& msg) : CastError(msg)
    {}
};

class CastErrorNDArray : public CastError {
public:
    explicit CastErrorNDArray(const std::string& msg) : CastError(msg)
    {}
};

/*
 * Abstract class for MsgpackObject and NDArray.
 */
class Object {

protected:
    // size of the flattened array, 0 for scalar data and NIL
    std::size_t size_;
    // data type, if container, it refers to the data type in the container
    std::string dtype_;

public:
    Object() : size_(0) {};
    virtual ~Object() = default;

    virtual std::size_t size() const = 0;
    virtual std::string dtype() const = 0;
    // empty vector for scalar data
    virtual std::vector<std::size_t> shape() const = 0;
    // empty string for scalar data
    virtual std::string containerType() const = 0;
};

/*
 * A container that holds a msgpack::object for deferred unpack.
 */
class MsgpackObject : public Object {

    msgpack::object value_; // msgpack::object has a shallow copy constructor

public:
    MsgpackObject() = default;  // must be default constructable

    explicit MsgpackObject(const msgpack::object& value): value_(value) {
        if (value.type == msgpack::type::object_type::ARRAY
                || value.type == msgpack::type::object_type::MAP
                || value.type == msgpack::type::object_type::BIN)
            size_ = value.via.array.size;

        if (value.type == msgpack::type::object_type::ARRAY)
            if (value.via.array.ptr)
                dtype_ = getTypeString(value.via.array.ptr[0].type);
            else
                dtype_ = "unknown";
        else if (value.type == msgpack::type::object_type::BIN)
            dtype_ = "char";
        else if (value.type == msgpack::type::object_type::MAP
               || value.type == msgpack::type::object_type::EXT)
            dtype_ = "undefined";
        else dtype_ = getTypeString(value.type);
    }

    ~MsgpackObject() override = default;

    MsgpackObject(const MsgpackObject&) = default;
    MsgpackObject& operator=(const MsgpackObject&) = default;

    MsgpackObject(MsgpackObject&&) = default;
    MsgpackObject& operator=(MsgpackObject&&) = default;

    /*
     * Cast the held msgpack::object to a given type.
     *
     * Exceptions:
     * CastErrorMsgpackObject: if cast fails
     */
    template<typename T>
    T as() const {
        try {
            return value_.as<T>();
        } catch(std::bad_cast& e) {
            std::string error_msg;
            if (size_)
                error_msg = ("The expected type is a(n) " + containerType() + " of " + dtype());
            else error_msg = ("The expected type is " + dtype());

            throw CastErrorMsgpackObject(error_msg);
        }
    }

    std::string dtype() const override { return dtype_; }

    std::size_t size() const override { return size_; }

    std::vector<std::size_t> shape() const override {
        if (size_) return std::vector<std::size_t>({size_});
        return std::vector<std::size_t>();
    }

    std::string containerType() const override {
        if (!size_) return ""; // scalar and NIL
        if (value_.type == msgpack::type::object_type::ARRAY
                || value_.type == msgpack::type::object_type::BIN)
            return "array-like";
        if (value_.type == msgpack::type::object_type::MAP) return "map";
        return getTypeString(value_.type);
    }

private:
    // map msgpack object types to strings
    static std::string getTypeString(msgpack::type::object_type type) {
        const std::map<msgpack::type::object_type, std::string> map {
            {msgpack::type::object_type::NIL, "MSGPACK_OBJECT_NIL"},
            {msgpack::type::object_type::BOOLEAN, "bool"},
            {msgpack::type::object_type::POSITIVE_INTEGER, "uint64_t"},
            {msgpack::type::object_type::NEGATIVE_INTEGER, "int64_t"},
            {msgpack::type::object_type::FLOAT32, "float"},
            {msgpack::type::object_type::FLOAT64, "double"},
            {msgpack::type::object_type::STR, "string"},
            {msgpack::type::object_type::ARRAY, "MSGPACK_OBJECT_ARRAY"},
            {msgpack::type::object_type::MAP, "MSGPACK_OBJECT_MAP"},
            {msgpack::type::object_type::BIN, "MSGPACK_OBJECT_BIN"},
            {msgpack::type::object_type::EXT, "MSGPACK_OBJECT_EXT"}
      };

      return map.at(type);
    }
};

using ObjectMap = std::map<std::string, MsgpackObject>;
using ObjectPair = std::pair<std::string, MsgpackObject>;

namespace detail {

template<typename Container, typename ElementType>
struct as_imp {
    Container operator()(void* ptr_, std::size_t size) {
        auto ptr = reinterpret_cast<const ElementType*>(ptr_);
        return Container(ptr, ptr + size);
    }
};

// partial specialization for std::array
template<typename ElementType, std::size_t N>
struct as_imp<std::array<ElementType, N>, ElementType> {
    std::array<ElementType, N> operator()(void*ptr_, std::size_t size) {
        if (size != N)
            throw CastErrorNDArray("The input size " + std::to_string(N) +
                                   " is different from the expected size " +
                                   std::to_string(size));
        auto ptr = reinterpret_cast<const ElementType*>(ptr_);
        std::array<ElementType, N> arr;
        memcpy(arr.data(), ptr, size * sizeof(ElementType));
        return arr;
    }
};

}  // detail

/*
 * A container held a pointer to the data chunk and other useful information.
 */
class NDArray : public Object {

    void* ptr_; // pointer to the data chunk
    std::vector<std::size_t> shape_; // shape of the array

public:
    NDArray() = default;

    // shape and dtype should be moved into the constructor
    NDArray(void* ptr, const std::vector<std::size_t>& shape, const std::string& dtype):
            ptr_(ptr), shape_(shape) {
        std::size_t size = 1;
        // Overflow is not expected since otherwise zmq::message_t
        // cannot hold the data.
        for (auto& v : shape) size *= v;
        size_ = size;
        dtype_ = dtype;
    }

    ~NDArray() override = default;

    NDArray(const NDArray&) = default;
    NDArray& operator=(const NDArray&) = default;

    NDArray(NDArray&&) = default;
    NDArray& operator=(NDArray&&) = default;

    std::size_t size() const override { return size_; }

    /*
     * Copy the data into a vector.
     *
     * Exceptions:
     * TypeMismatchErrorNDArray: if type mismatches
     * CastErrorNDArray: if cast fails
     */
    template<typename Container,
             typename = typename std::enable_if<!std::is_integral<Container>::value>::type>
    Container as() const {
        typedef typename Container::value_type ElementType;
        if (!validateType<ElementType>(dtype_))
            throw TypeMismatchErrorNDArray("The expected type is a(n) " +
                                           containerType() + " of " + dtype());
        detail::as_imp<Container, ElementType> as_imp_instance;
        return as_imp_instance(ptr_, size());
    }

    std::vector<std::size_t> shape() const override { return shape_; }

    std::string dtype() const override { return dtype_; }

    std::string containerType() const override { return "array-like"; }

    /*
     * Return a casted pointer to the held array data.
     *
     * Exceptions:
     * TypeMismatchErrorNDArray: if type mismatches
     */
    template<typename T>
    T* data() const {
        if (!validateType<T>(dtype_))
            throw TypeMismatchErrorNDArray("The expected pointer type is " + dtype());
        return reinterpret_cast<T*>(ptr_);
    }

    // Return a void pointer to the held array data.
    void* data() const { return ptr_; }

private:
    /*
     * Use to check data type before casting an NDArray object.
     *
     * Implicit type conversion is not allowed.
     */
    template <typename T>
    static bool validateType(const std::string& type_string) {
        if (type_string == "uint64_t" && std::is_same<T, uint64_t>::value) return true;
        if (type_string == "uint32_t" && std::is_same<T, uint32_t>::value) return true;
        if (type_string == "uint16_t" && std::is_same<T, uint16_t>::value) return true;
        if (type_string == "uint8_t" && std::is_same<T, uint8_t>::value) return true;
        if (type_string == "int64_t" && std::is_same<T, int64_t>::value) return true;
        if (type_string == "int32_t" && std::is_same<T, int32_t>::value) return true;
        if (type_string == "int16_t" && std::is_same<T, int16_t>::value) return true;
        if (type_string == "int8_t" && std::is_same<T, int8_t>::value) return true;
        if (type_string == "float" && std::is_same<T, float>::value) return true;
        if (type_string == "double" && std::is_same<T, double>::value) return true;
        return (type_string == "bool" && std::is_same<T, bool>::value);
    }
};

} // karabo_bridge


namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor{

/*
 * template specialization for karabo_bridge::object
 */
template<>
struct as<karabo_bridge::MsgpackObject> {
    karabo_bridge::MsgpackObject operator()(msgpack::object const& o) const {
        return karabo_bridge::MsgpackObject(o.as<msgpack::object>());
    }
};

} // adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // msgpack


namespace karabo_bridge {

/*
 * Data structure presented to the user.
 *
 * - The data member "metadata" holds a map of meta data;
 * - The data member "array" holds a map of array data, which is usually a
 *   big chunk of data;
 * - The data member "data_" holds a map of normal data, which can be either
 *   scalar data or small arrays.
 */
class KbData {

    ObjectMap data_;
    std::vector<zmq::message_t> mpmsg_; // maintain the lifetime of data
    std::vector<msgpack::object_handle> handles_; // maintain the lifetime of data

public:

    KbData() = default;

    ~KbData() = default;

    KbData(const KbData&) = delete;
    KbData& operator=(const KbData&) = delete;

    KbData(KbData&&) = default;
    KbData& operator=(KbData&&) = default;

    using iterator = ObjectMap::iterator;
    using const_iterator = ObjectMap::const_iterator;

    ObjectMap metadata;
    std::map<std::string, NDArray> array;

    MsgpackObject& operator[](const std::string& key) { return data_.at(key); }

    iterator begin() noexcept { return data_.begin(); }
    iterator end() noexcept { return data_.end(); }
    const_iterator begin() const noexcept { return data_.begin(); }
    const_iterator end() const noexcept { return data_.end(); }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    const_iterator cend() const noexcept { return data_.cend(); }

    template<typename T>
    std::pair<iterator, bool> insert(T&& value) {
        return data_.insert(std::forward<T>(value));
    }

    std::size_t bytesReceived() const {
        std::size_t size_ = 0;
        for (auto& m : mpmsg_) size_ += m.size();
        return size_;
    }

    void appendMsg(zmq::message_t&& msg) {
        mpmsg_.push_back(std::move(msg));
    }

    void appendHandle(msgpack::object_handle&& oh) {
        handles_.push_back(std::move(oh));
    }

    void swap(KbData& other) {
        metadata.swap(other.metadata);
        array.swap(other.array);
        data_.swap(other.data_);
        mpmsg_.swap(other.mpmsg_);
        handles_.swap(other.handles_);
    }
};

} // karabo_bridge

#endif //KARABO_BRIDGE_KB_DATA_HPP
