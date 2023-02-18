#include "so.hpp"

#include <dlfcn.h>

using namespace std;

SharedObject::~SharedObject()
{
    unload();
}

bool SharedObject::load(const string &path)
{
	dlerror();
	handle = dlopen(path.c_str(), RTLD_NOW);
	if (!handle) {
        const char* err = dlerror();
        error = err ? err : "unknown error";
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