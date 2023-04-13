#include "ax25header.hpp"

#include <algorithm>
#include <sstream>
#include <cassert>

using namespace std;

namespace AX25Base {

AX25Header::AX25Header(OctetArray frame):
    destination{L2Callsign(frame, 0, last)},
    command{destination.get_chBit()},
    source{L2Callsign(frame, 7, last)},
    response{source.get_chBit()}
{
    if (last)
    {
        this->digis = vector<L2Callsign>();
        this->length = 14;
        return;
    }
    int i;
    for (i = 14; !last; i += 7)
        digis.push_back(L2Callsign(frame, i, last));
    this->length = i;
}

AX25Header::AX25Header(const L2Callsign& _source, const L2Callsign& _destination,
           const std::vector<L2Callsign>& digis):
    source{_source}, response{_source.get_chBit()},
    destination{_destination}, command{_destination.get_chBit()}
{
    this->digis = digis;
    this->length = 14 + ( 7 * digis.size() );
}

AX25Header::AX25Header(AX25Header::Ptr _template, bool _command, bool _response):
    source{_template->Source()}, response{_response},
    destination{_template->Destination()}, command{_command}
{
    this->digis = _template->Digis();
    this->length = _template->Length();
}

const L2Callsign& AX25Header::NextHop() const
{
    auto i = find_if(digis.begin(), digis.end(), [] (const L2Callsign& cs) -> bool {
        return !cs.get_chBit();
    });
    return (i != digis.end() ? *i : destination);
}

bool AX25Header::MustDigi() const
{
    auto i = find_if(digis.begin(), digis.end(), [] (const L2Callsign& cs) -> bool {
        return !cs.get_chBit();
    });
    return (i != digis.end());
}

void AX25Header::Digipeat()
{
    for (int i = 0; i < digis.size(); ++i)
    {
        const L2Callsign& hop = digis[i];
        if (!hop.get_chBit())
        {
            digis[i] = L2Callsign(hop, true);
            return;
        }
    } // end for //
    throw new runtime_error("All digis are already passed");
}

OctetArray AX25Header::GetOctets() const
{
    int nDigis = digis.size();
    OctetArray frame(new octet_vector_t());
    FillInOctetArray(frame);
    return frame;
}

void AX25Header::FillInOctetArray(OctetArray& frame) const
{
    assert(frame);
    frame->clear();
    auto dst{destination.octets()};
    frame->insert(frame->end(), dst->begin(), dst->end());
    auto src{source.octets()};
    frame->insert(frame->end(), src->begin(), src->end());
    for_each(digis.begin(), digis.end(), [&frame] (const L2Callsign& cs) {
        OctetArray f1{cs.octets()};
        frame->insert(frame->end(), f1->begin(), f1->end());
    });
    frame->at(frame->size() - 1) |= 0x01; // SDLC end bit
}

std::string AX25Header::ToString() const
{
    ostringstream sb;
    sb << source.to_string() << " -> " << destination.to_string();
    if (!digis.empty()) {
        sb << " via";
        for_each(digis.begin(), digis.end(), [&sb] (const L2Callsign& cs) {
            sb << " " << cs.to_string();
            if (cs.get_chBit())
                sb << "*";
        });
    }
    return sb.str();
}

std::vector<L2Callsign> AX25Header::DigisReversed() const
{
    if (digis.empty())
        return digis;
    std::vector<L2Callsign> result;
    result.reserve(digis.size());
    for_each(digis.rbegin(), digis.rend(), [&result] (const L2Callsign& cs) {
        result.push_back(L2Callsign(cs, false));
    });
    return result;
}

} // namespace AX25Base
