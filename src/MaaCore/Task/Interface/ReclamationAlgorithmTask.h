#pragma once
#include "Task/InterfaceTask.h"

namespace asst
{
    class ProcessTask;

    class ReclamationAlgorithmTask : public InterfaceTask
    {
    public:
        inline static constexpr std::string_view TaskType = "ReclamationAlgorithm";

        ReclamationAlgorithmTask(const AsstCallback& callback, Assistant* inst);
        virtual ~ReclamationAlgorithmTask() override = default;

        virtual bool set_params(const json::value& params) override;

    private:
        std::shared_ptr<ProcessTask> m_reclamation_algorithm_task_ptr = nullptr;
    };
}
