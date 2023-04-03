#ifndef _BK_BASE_BKOBJECT_H
#define _BK_BASE_BKOBJECT_H

#include <string>
#include <stdexcept>
#include <cassert>

namespace BkBase {

    class BkObject {
    public:
        BkObject() = default;
        BkObject(const BkObject& other) = delete;
        BkObject(BkObject&& other) = delete;

    protected:
        virtual std::string name() const = 0;
    };

    class BkInvalidCastException: public std::runtime_error {
    public:
        BkInvalidCastException(const std::string& msg):
            std::runtime_error("Invalid Cast: " + msg)
        {}
    };

    template<typename T>
    T& self(void* p) {
        auto _p{static_cast<BkObject*>(p)};
        if (!_p)
            throw BkInvalidCastException(
                std::string("BkBase::self(p) for ") + typeid(T).name() +
                std::string(" is null"));
        auto _self{dynamic_cast<T*>(_p)};
        if (!_self)
            throw BkInvalidCastException(
                std::string("BkBase::self(p) for ") + typeid(T).name() +
                std::string(" <-> ") + typeid(p).name());
        return *_self;
    };

} // end namespace BkBase //

#endif //_BK_BASE_BKOBJECT_H//
