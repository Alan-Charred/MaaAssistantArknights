#pragma once

#include "AbstractReclamationTaskPlugin.h"

#include "Task/BattleHelper.h"

namespace asst
{
class ReclamationBattleTaskPlugin : public AbstractReclamationTaskPlugin, private BattleHelper
{
public:
    ReclamationBattleTaskPlugin(
        const AsstCallback& callback,
        Assistant* inst,
        std::string_view task_chain,
        const std::shared_ptr<ReclamationConfig>& config) :
        AbstractReclamationTaskPlugin(callback, inst, task_chain, config),
        BattleHelper(inst) {};
    virtual ~ReclamationBattleTaskPlugin() override = default;
    virtual bool verify(AsstMsg msg, const json::value& details) const override;
    virtual bool load_params(const json::value& params) override;

protected:
    virtual bool _run() override;

    virtual AbstractTask& this_task() override { return *this; }

    virtual void clear() override;

private:
    bool calc_stage_info();
    bool do_once();
    bool deploy_oper(
        int x,
        int y,
        int oper_x,
        int oper_y,
        int target_x,
        int target_y,
        battle::DeployDirection direction,
        bool camera_move = false,
        int direct_x = 0,
        int direct_y = 0);
    bool retreat_oper(
        const int& oper_x,
        const int& oper_y,
        const int& retreat_x,
        const int& retreat_y,
        const bool& camera_move = false);

    // ———————— constants and variables ———————————————————————————————————————————————
    int m_strategy = 4;
    int m_step = 0;
    bool m_first_deploy = true;
};
} // namespace asst
