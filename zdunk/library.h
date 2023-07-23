#pragma once
#include <memory>
#include "module.h"

namespace zdunk
{

    class Library
    {
    public:
        static Module::ptr GetModule(const std::string &path);
    };

}
