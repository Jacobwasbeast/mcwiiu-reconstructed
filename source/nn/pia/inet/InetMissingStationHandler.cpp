#include "nn/pia/inet/InetMissingStationHandler.hpp"

namespace nn { namespace pia { namespace inet {

InetMissingStationHandler::~InetMissingStationHandler() {}
unsigned int InetMissingStationHandler::Execute(transport::StationConnectionInfo*)
{
    return 0;
}

}}}
