#include "ReclamationAlgorithmBattleTaskPlugin.h"

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/OCRer.h"
#include "Vision/RegionOCRer.h"
#include "Vision/Matcher.h"

#include "Config/Miscellaneous/BattleDataConfig.h"
#include "Config/Miscellaneous/TilePack.h"

#include <algorithm>

bool asst::ReclamationAlgorithmBattleTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskCompleted || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    const auto task_name = details.get("details", "task", "");

    if (task_name == "ReclamationAlgorithm@StartAction") {
        return true;
     }

     return false;
}

bool asst::ReclamationAlgorithmBattleTaskPlugin::_run()
{
    using namespace std::chrono_literals;
    
    if (!calc_stage_info()) {
        return false;
    }
    wait_until_start(false);
    // speed_up();

    while (true) {
        // 不在战斗场景，且已使用过了干员，说明已经打完了，就结束循环
        if (!do_once() && !m_first_deploy) {
            break;
        }
    }
    return true;
}

bool asst::ReclamationAlgorithmBattleTaskPlugin::calc_stage_info()
{
    wait_for_start_button_clicked();
    clear();

    bool calced = false;

    const auto stage_name_task_ptr = Task.get("BattleStageName");
    sleep(stage_name_task_ptr->pre_delay);

    auto start = std::chrono::steady_clock::now();
    constexpr auto kTimeout = std::chrono::seconds(20);

    while (std::chrono::steady_clock::now() - start < kTimeout) {
        if (need_exit()) {
            return false;
        }
        RegionOCRer name_analyzer(ctrler()->get_image());
        name_analyzer.set_task_info(stage_name_task_ptr);
        if (!name_analyzer.analyze()) {
            continue;
        }
        const std::string& text = name_analyzer.get_result().text;
        if (text.empty()) {
            continue;
        }

        calced = true;
        m_stage_name = text;

        if (calced) {
            break;
        }
        std::this_thread::yield();
    }

    if (!calced) {
        callback(AsstMsg::SubTaskExtraInfo, basic_info_with_what("StageInfoError"));
        return false;
    }

    auto cb_info = basic_info_with_what("StageInfo");
    auto& details = cb_info["details"];
    details["name"] = m_stage_name;
    callback(AsstMsg::SubTaskExtraInfo, cb_info);

    return true;
}

bool asst::ReclamationAlgorithmBattleTaskPlugin::do_once()
{
    sleep(800);

    cv::Mat image = ctrler()->get_image();
    
    Matcher matcher(image);
    matcher.set_task_info("ReclamationAlgorithm@InBattle");
    if (!matcher.analyze()) {
        clp_pd_callback("已不在战斗中");
        return false;
    }
    update_cost(image);

    switch (m_step) {
        case 0:
            if (m_cost < 6) {
                return true;
            }
            // clp_pd_callback("3级源石虫引诱器械");
            deploy_oper(1000, 660, 977, 660, 670, 365, DeployDirection::None);
            break;
        case 1:
            if (m_cost < 6) {
                return true;
            }
            // clp_pd_callback("2级源石虫引诱器械");
            deploy_oper(1080, 660, 1050, 660, 670, 435, DeployDirection::None);
            break;
        case 2:
            if (m_cost < 6) {
                return true;
            }
            // clp_pd_callback("1级源石虫引诱器械");
            deploy_oper(1160, 660, 1169, 660, 675, 515, DeployDirection::None, true);
            break;
        case 3:
            if (m_cost < 9) {
                return true;
            }
            // clp_pd_callback("极境");
            deploy_oper(680, 660, 687, 660, 615, 280, DeployDirection::Right);
            break;
        case 4:
            if (m_cost < 20) {
                return true;
            }
            // clp_pd_callback("阿斯卡伦");
            deploy_oper(900, 660, 854, 660, 455, 425, DeployDirection::Right, true, 590, 355);
            break;
        case 5:
            if (m_cost < 5) {
                return true;
            }
            // clp_pd_callback("砾");
            deploy_oper(46, 660, 59, 660, 520, 380, DeployDirection::Left, true, 620, 335);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 6:
            if (m_cost < 5) {
                return true;
            }
            // clp_pd_callback("孑");
            deploy_oper(138, 660, 178, 660, 625, 350, DeployDirection::Right);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 7:
            if (m_cost < 5) {
                return true;
            }
            // clp_pd_callback("夜刀");
            deploy_oper(231, 660, 297, 660, 625, 350, DeployDirection::Left);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 8:
            if (m_cost < 7) {
                return true;
            }
            // clp_pd_callback("红");
            deploy_oper(323, 660, 359, 660, 625, 350, DeployDirection::Right);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 9:
            if (m_cost < 8) {
                return true;
            }
            // clp_pd_callback("缄默德克萨斯");
            deploy_oper(415, 650, 432, 660, 625, 350, DeployDirection::Left);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 10:
            if (m_cost < 9) {
                return true;
            }
            // clp_pd_callback("麒麟夜刀");
            deploy_oper(508, 660, 504, 660, 625, 350, DeployDirection::Right);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 11:
            if (m_cost < 9) {
                return true;
            }
            // clp_pd_callback("傀影");
            deploy_oper(600, 660, 577, 660, 625, 350, DeployDirection::Left);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 12:
            if (m_cost < 9) {
                return true;
            }
            // clp_pd_callback("伊内斯");
            deploy_oper(692, 660, 650, 660, 625, 350, DeployDirection::Right);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 13:
            if (m_cost < 11) {
                return true;
            }
            // clp_pd_callback("艾拉");
            deploy_oper(785, 660, 722, 660, 625, 350, DeployDirection::Up);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 14:
            if (m_cost < 6) {
                return true;
            }
            // clp_pd_callback("3级源石虫引诱器械");
            deploy_oper(969, 660, 903, 660, 695, 215, DeployDirection::None);
            break;
        case 15:
            if (m_cost < 6) {
                return true;
            }
            // clp_pd_callback("2级源石虫引诱器械");
            deploy_oper(1062, 660, 1022, 660, 700, 280, DeployDirection::None);
            break;
        case 16:
            if (m_cost < 6) {
                return true;
            }
            // clp_pd_callback("1级源石虫引诱器械");
            deploy_oper(1154, 660, 1141, 660, 775, 270, DeployDirection::None);
            break;
        case 17:
            if (m_cost < 7) {
                return true;
            }
            // clp_pd_callback("砾");
            deploy_oper(46, 660, 59, 660, 625, 350, DeployDirection::Left);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 18:
            if (m_cost < 7) {
                return true;
            }
            // clp_pd_callback("孑");
            deploy_oper(138, 660, 178, 660, 625, 350, DeployDirection::Right);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 19:
            if (m_cost < 10) {
                return true;
            }
            // clp_pd_callback("红");
            deploy_oper(323, 660, 359, 660, 625, 350, DeployDirection::Left);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 20:
            if (m_cost < 12) {
                return true;
            }
            // clp_pd_callback("缄默德克萨斯");
            deploy_oper(415, 660, 432, 660, 625, 350, DeployDirection::Right);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 21:
            if (m_cost < 13) {
                return true;
            }
            // clp_pd_callback("麒麟夜刀");
            deploy_oper(508, 660, 504, 660, 625, 350, DeployDirection::Left);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 22:
            if (m_cost < 13) {
                return true;
            }
            // clp_pd_callback("傀影");
            deploy_oper(600, 660, 577, 660, 625, 350, DeployDirection::Right);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 23:
            if (m_cost < 13) {
                return true;
            }
            // clp_pd_callback("伊内斯");
            deploy_oper(692, 660, 650, 660, 625, 350, DeployDirection::Left);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        case 24:
            if (m_cost < 7) {
                return true;
            }
            // clp_pd_callback("夜刀");
            deploy_oper(231, 660, 297, 660, 625, 350, DeployDirection::Right);
            sleep(500);
            retreat_oper(545, 335, 605, 230);
            break;
        default:
            switch ((m_step - 25) % 7) {
                case 0:
                    if (m_cost < 10) {
                        return true;
                    }
                    // clp_pd_callback("砾");
                    deploy_oper(46, 660, 59, 660, 625, 350, DeployDirection::Left);
                    sleep(500);
                    retreat_oper(545, 335, 605, 230);
                    break;
                case 1:
                    if (m_cost < 10) {
                        return true;
                    }
                    // clp_pd_callback("孑");
                    deploy_oper(138, 660, 178, 660, 625, 350, DeployDirection::Right);
                    sleep(500);
                    retreat_oper(545, 335, 605, 230);
                    break;
                case 2:
                    if (m_cost < 14) {
                        return true;
                    }
                    // clp_pd_callback("红");
                    deploy_oper(323, 660, 359, 660, 625, 350, DeployDirection::Left);
                    sleep(500);
                    retreat_oper(545, 335, 605, 230);
                    break;
                case 3:
                    if (m_cost < 16) {
                        return true;
                    }
                    // clp_pd_callback("缄默德克萨斯");
                    deploy_oper(415, 660, 432, 660, 625, 350, DeployDirection::Right);
                    sleep(500);
                    retreat_oper(545, 335, 605, 230);
                    break;
                case 4:
                    if (m_cost < 18) {
                        return true;
                    }
                    // clp_pd_callback("麒麟夜刀");
                    deploy_oper(508, 660, 504, 660, 625, 350, DeployDirection::Left);
                    sleep(500);
                    retreat_oper(545, 335, 605, 230);
                    break;
                case 5:
                    if (m_cost < 18) {
                        return true;
                    }
                    // clp_pd_callback("傀影");
                    deploy_oper(600, 660, 577, 660, 625, 350, DeployDirection::Left);
                    sleep(500);
                    retreat_oper(545, 335, 605, 230);
                    break;
                case 6:
                    if (m_cost < 18) {
                        return true;
                    }
                    // clp_pd_callback("伊内斯");
                    deploy_oper(692, 660, 650, 660, 625, 350, DeployDirection::Right);
                    sleep(500);
                    retreat_oper(545, 335, 605, 230);
                    break;
            }
    }
    m_first_deploy = false;
    ++m_step;
    
    return true;
}

void asst::ReclamationAlgorithmBattleTaskPlugin::clp_pd_callback(std::string cur, int deepen_or_weaken, std::string prev)
{
    callback(AsstMsg::SubTaskExtraInfo, json::object { 
        { "what", "RoguelikeCollapsalParadigms" },
        { "details", json::object {
            { "cur", cur },
            { "deepen_or_weaken", deepen_or_weaken },
            { "prev", prev }
        } }
    });
}

void asst::ReclamationAlgorithmBattleTaskPlugin::wait_for_start_button_clicked()
{
    ProcessTask(*this, { "ReclamationAlgorithm@WaitForStartButtonClicked" })
        .set_task_delay(0)
        .set_retry_times(0)
        .run();
}

void asst::ReclamationAlgorithmBattleTaskPlugin::clear()
{
    BattleHelper::clear();
    m_stage_name.clear();
    m_first_deploy = true;
    m_step = 0;
}

bool asst::ReclamationAlgorithmBattleTaskPlugin::deploy_oper(int x, int y, int oper_x, int oper_y, int target_x, int target_y, DeployDirection direction, bool camera_move, int direct_x, int direct_y) {
    const auto swipe_oper_task_ptr = Task.get("BattleSwipeOper");
    const auto use_oper_task_ptr = Task.get("BattleUseOper");
    
    Point target_point(target_x, target_y);

    int dist = static_cast<int>(Point::distance(
        target_point,
        { oper_x, oper_y }));
    
    // 1000 是随便取的一个系数，把整数的 pre_delay 转成小数用的
    int duration = static_cast<int>(dist / 1200.0 * swipe_oper_task_ptr->pre_delay) + 300;
    // 时间太短了的压根放不上去，故意加长一点
    if (int min_duration = swipe_oper_task_ptr->special_params.at(4) + 300; duration < min_duration) {
        duration = min_duration;
    }
    bool deploy_with_pause = ControlFeat::support(
        ctrler()->support_features(),
        ControlFeat::SWIPE_WITH_PAUSE);
    Point click_point(x, y);
    ctrler()->click(click_point);
    sleep(use_oper_task_ptr->pre_delay + 200);
    Point oper_point(oper_x, oper_y);
    ctrler()->swipe(
        oper_point,
        target_point,
        duration,
        false,
        swipe_oper_task_ptr->special_params.at(2),
        swipe_oper_task_ptr->special_params.at(3),
        deploy_with_pause);

    // 拖动干员朝向
    sleep(use_oper_task_ptr->post_delay);
    if (camera_move) {
        sleep(500);
        target_point.x = direct_x;
        target_point.y = direct_y;
    }
    if (direction != DeployDirection::None) {
        static const std::unordered_map<DeployDirection, Point> DirectionMap = {
            { DeployDirection::Right, Point::right() }, { DeployDirection::Down, Point::down() },
            { DeployDirection::Left, Point::left() },   { DeployDirection::Up, Point::up() },
            { DeployDirection::None, Point::zero() },
        };

        // 计算往哪边拖动
        const Point& direction_target = DirectionMap.at(direction);

        // 将方向转换为实际的 swipe end 坐标点，并对滑动距离进行缩放
        const auto scale_size = ctrler()->get_scale_size();
        static const int coeff =
            static_cast<int>(swipe_oper_task_ptr->special_params.at(0) * scale_size.second / 720.0);
        Point end_point = target_point + (direction_target * coeff);

        fix_swipe_out_of_limit(
            target_point,
            end_point,
            scale_size.first,
            scale_size.second,
            swipe_oper_task_ptr->special_params.at(1));

        sleep(use_oper_task_ptr->post_delay);
        ctrler()->swipe(target_point, end_point, swipe_oper_task_ptr->post_delay);
        // 仅简单复用，该延迟含义与此处逻辑无关 by MistEO
        sleep(use_oper_task_ptr->pre_delay + 100);
    }

    if (deploy_with_pause) {
        // m_inst_helper.ctrler()->press_esc();
        ProcessTask(this_task(), { "BattlePause" }).run();
    }
    
    return true;
}

bool asst::ReclamationAlgorithmBattleTaskPlugin::retreat_oper(int oper_x, int oper_y, int retreat_x, int retreat_y, bool camera_move)
{
    Point oper_point(oper_x, oper_y);
    ctrler()->click(oper_point);

    sleep(500);
    if (camera_move) {
        sleep(500);
    }

    Point retreat_point(retreat_x, retreat_y);
    ctrler()->click(retreat_point);
    
    return true;
}
