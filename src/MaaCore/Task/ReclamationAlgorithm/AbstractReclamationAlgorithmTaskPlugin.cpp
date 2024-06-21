#include "AbstractReclamationAlgorithmTaskPlugin.h"

asst::AbstractReclamationAlgorithmTaskPlugin::AbstractReclamationAlgorithmTaskPlugin(
    const AsstCallback& callback, Assistant* inst, std::string_view task_chain)
    : AbstractTaskPlugin(callback, inst, task_chain)
{}
