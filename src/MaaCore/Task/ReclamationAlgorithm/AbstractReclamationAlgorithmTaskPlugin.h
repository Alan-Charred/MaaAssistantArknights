#pragma once

#include "Task/AbstractTaskPlugin.h"

namespace asst
{
    class AbstractReclamationAlgorithmTaskPlugin : public AbstractTaskPlugin
    {
    public:
        AbstractReclamationAlgorithmTaskPlugin(const AsstCallback& callback, Assistant* inst, std::string_view task_chain);
    };
}
