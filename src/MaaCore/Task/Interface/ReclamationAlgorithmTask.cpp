#include "ReclamationAlgorithmTask.h"

#include "Common/AsstBattleDef.h"
#include "Config/TaskData.h"
#include "Status.h"
#include "Task/ProcessTask.h"
#include "Task/ReclamationAlgorithm/ReclamationAlgorithmBattleTaskPlugin.h"
#include "Task/ReclamationAlgorithm/ReclamationAlgorithmPerfectSlugTaskPlugin.h"

#include "Utils/Logger.hpp"

asst::ReclamationAlgorithmTask::ReclamationAlgorithmTask(const AsstCallback& callback, Assistant* inst)
    : InterfaceTask(callback, inst, TaskType),
      m_reclamation_algorithm_task_ptr(std::make_shared<ProcessTask>(callback, inst, TaskType))
{
    LogTraceFunction;

    m_reclamation_algorithm_task_ptr->set_ignore_error(true);
    m_reclamation_algorithm_task_ptr->register_plugin<ReclamationAlgorithmBattleTaskPlugin>();
    m_reclamation_algorithm_task_ptr->register_plugin<ReclamationAlgorithmPerfectSlugTaskPlugin>();

    m_reclamation_algorithm_task_ptr->set_tasks({ "ReclamationAlgorithm@Begin" });
    
    m_subtasks.emplace_back(m_reclamation_algorithm_task_ptr);;
}

bool asst::ReclamationAlgorithmTask::set_params([[maybe_unused]] const json::value& params)
{
    LogTraceFunction;

    return true;
}
