#ifndef _SO_HPP
#define _SO_HPP

#include <string>
#include <memory>

class SharedObject {
public:
    typedef std::shared_ptr<SharedObject> Ptr_t;
    
    SharedObject() = default;
    SharedObject(const SharedObject& other) = delete;
    SharedObject(SharedObject&& other) = delete;
    ~SharedObject();

    bool load(const std::string &path);
    void unload();
    void* getsym(const std::string &name);

    std::string error_text() const { return error; }

private:
    void* handle{nullptr};
    std::string error{};
};

#endif // _SO_HPP //