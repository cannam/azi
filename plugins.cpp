
#include <vamp/vamp.h>
#include <vamp-sdk/PluginAdapter.h>

#include "Azi.h"

static Vamp::PluginAdapter<Azi> myPluginAdapter;

const VampPluginDescriptor *
vampGetPluginDescriptor(unsigned int version, unsigned int index)
{
    if (version < 1) return 0;

    switch (index) {
    case  0: return myPluginAdapter.getDescriptor();
    default: return 0;
    }
}


