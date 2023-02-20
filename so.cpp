#include "so.hpp"
#include "service.hpp"

#include <dlfcn.h>
#include <uuid/uuid.h>

using namespace std;
using namespace jsonx;

static string new_uuid() {
    uuid_t id;
    uuid_generate(id);
    char out[40];
    uuid_unparse_lower(id, out);
    return out;
}

SharedObject::Map_t SharedObject::container;

SharedObject::Ptr_t SharedObject::create(filesystem::path path, json meta)    
{
    auto ptr = Ptr_t(new SharedObject(meta));
    if (!ptr->load(path))
        throw SharedObjectException("Unable to load plugin from: " 
                                        + string(path.c_str()));
    return container.emplace(ptr->id, ptr).first->second;
}

const SharedObject& SharedObject::create_so(filesystem::path path, json meta) {
    auto ptr = Ptr_t(new SharedObject(meta));
    if (!ptr->load(path))
        throw SharedObjectException("Unable to load plugin from: " 
                                        + string(path.c_str()));
    return *container.emplace(ptr->id, ptr).first->second;
}

SharedObject::SharedObject(json _meta): 
    id{new_uuid()}, meta{_meta}
{
}

SharedObject::~SharedObject()
{
    unload();
}

bool SharedObject::load(const string &path)
{
    // Load the shared object;
	dlerror();
	handle = dlopen(path.c_str(), RTLD_NOW);
	if (!handle) {
        const char* err = dlerror();
        error = err ? err : "unknown error";
        cerr << "[f] " << error << endl;
        return false;
	}
	return true;
}

void SharedObject::unload()
{
    if (handle) {
        dlclose(handle);
        handle = nullptr;
    }
}

void* SharedObject::getsym(const string &name)
{
    dlerror();
    void* addr = dlsym(handle, name.c_str());
	if (!addr) {
        const char* err = dlerror();
        error = err ? err : "unknown error";
        return nullptr;
	}
	return addr;
}

bk_error_t SharedObject::start()
{
    return BK_ERC_OK;
}

bk_error_t SharedObject::stop()
{
    return BK_ERC_OK;
}

