
#include <daisy/module.h>
#include <daisy/service.h>

#include <iostream>
#include <string>
#include <cassert>

using namespace std;

static const service_t* sys_service = nullptr;

static void on_load(const service_t* _sys_service)
{
    cout << "Module loaded" << endl;
    assert(_sys_service);
    assert(_sys_service->publish);
    assert(_sys_service->withdraw);
    ::sys_service = _sys_service;
}

static void on_start(const char *meta)
{
    cout << "Module started" << endl;
}

static void on_stop()
{
    cout << "Module stopped" << endl;
}

extern "C" {
    module_t module {
        .load  = on_load, 
        .start = on_start, 
        .stop  = on_stop
    };
} // end extern C //
