#include "nn/nex/PseudoSingleton.hpp"

namespace nn
{
namespace nex
{

bool g_usingInstantiationContext = false;
ThreadVariable<unsigned int>* g_instantiationContextVariable = 0;

}
}
