#include "port.hpp"
#include "session.hpp"
#include "utils.hpp"

Port::Ptr_t Port::create(Session& session, int id, const jsonx::json &meta)
{
   return Ptr_t(new Port(session, id, meta));
}

void Port::receive(const jsonx::json &meta, const char *pb, size_t cb)
{
    switch (string_2_kind(meta["kind"])) {
    default:
        break;
    } // end switch //
}

Port::~Port()
{}

Port::Port(Session& _session, int _id, const jsonx::json &_meta):
    session{_session}, id{_id}, meta{_meta}
{}
