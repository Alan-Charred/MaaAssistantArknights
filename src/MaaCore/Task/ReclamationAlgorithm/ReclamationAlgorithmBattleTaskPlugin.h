#pragma once

#include "AbstractReclamationAlgorithmTaskPlugin.h"
#include "Common/AsstBattleDef.h"
#include "Common/AsstTypes.h"
#include "Config/Miscellaneous/TilePack.h"
#include "Task/BattleHelper.h"

using namespace asst::battle;

namespace asst
{
    class ReclamationAlgorithmBattleTaskPlugin : public AbstractReclamationAlgorithmTaskPlugin, private BattleHelper
    {
    public:
        ReclamationAlgorithmBattleTaskPlugin(const AsstCallback& callback, Assistant* inst, std::string_view task_chain)
            : AbstractReclamationAlgorithmTaskPlugin(callback, inst, task_chain), BattleHelper(inst) {};
        virtual ~ReclamationAlgorithmBattleTaskPlugin() override = default;

        virtual bool verify(AsstMsg msg, const json::value& details) const override;

    protected:
        virtual bool _run() override;
        virtual AbstractTask& this_task() override { return *this; }
        virtual void clear() override;

    private:
        int m_step = 0;
        bool m_first_deploy = true;

        bool calc_stage_info();
        void wait_for_start_button_clicked();
        bool do_once();
        bool deploy_oper(int x, int y, int oper_x, int oper_y, int target_x, int target_y, DeployDirection direction, bool camera_move = false, int direct_x = 0, int direct_y = 0);
        bool retreat_oper(int oper_x, int oper_y, int retreat_x, int retreat_y, bool camera_move = false);
        void clp_pd_callback(std::string cur, int deepen_or_weaken = 0, std::string prev = "");
    };
}
