#include "ReclamationBattleTaskPlugin.h"

#include <algorithm>
#include <chrono>

#include "Config/GeneralConfig.h"
#include "Config/Miscellaneous/BattleDataConfig.h"
#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/Matcher.h"
#include "Vision/OCRer.h"
#include "Vision/RegionOCRer.h"

bool asst::ReclamationBattleTaskPlugin::load_params([[maybe_unused]] const json::value& params)
{
    LogTraceFunction;

    if (const ReclamationMode& mode = m_config->get_mode(); mode != ReclamationMode::PerfectSlugs) {
        return false;
    }

    return true;
}

bool asst::ReclamationBattleTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    const std::string& task_name = details.get("details", "task", "");
    if (task_name == m_config->get_theme() + "@RA@PS-CaptureSlugs") {
        return true;
    }

    return false;
}

bool asst::ReclamationBattleTaskPlugin::_run()
{
    LogTraceFunction;

    using namespace std::chrono_literals;

    if (!calc_stage_info()) {
        return false;
    }

    wait_until_start(true);

    auto start_time = std::chrono::steady_clock::now();
    while (!need_exit()) {
        // 不在战斗场景，且已使用过了干员，说明已经打完了，就结束循环
        if (!do_once() && !m_first_deploy) {
            break;
        }

        auto duration = std::chrono::steady_clock::now() - start_time;
        if (duration > 10min) {
            m_task_ptr->set_enable(false);
            break;
        }

        sleep(100);
    }
    return true;
}

bool asst::ReclamationBattleTaskPlugin::calc_stage_info()
{
    LogTraceFunction;

    clear();

    bool calced = false;

    const auto stage_name_guard_task_ptr = Task.get("Tales@RA@OperationNameGuard");
    const auto stage_name_task_ptr = Task.get("BattleStageName");
    sleep(stage_name_task_ptr->pre_delay);

    auto start = std::chrono::steady_clock::now();
    constexpr auto kTimeout = std::chrono::seconds(20);

    while (std::chrono::steady_clock::now() - start < kTimeout) {
        if (need_exit()) {
            return false;
        }

        RegionOCRer name_analyzer(ctrler()->get_image());
        name_analyzer.set_task_info(stage_name_guard_task_ptr);
        if (!name_analyzer.analyze()) {
            continue;
        }
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

void asst::ReclamationBattleTaskPlugin::clear()
{
    BattleHelper::clear();
    m_stage_name.clear();
    m_first_deploy = true;
    m_step = 0;
}

bool asst::ReclamationBattleTaskPlugin::do_once()
{
    // TODO: move this to controller
    thread_local auto prev_frame_time = std::chrono::steady_clock::time_point {};
    static const auto min_frame_interval =
        std::chrono::milliseconds(Config.get_options().roguelike_fight_screencap_interval);

    // prevent our program from consuming too much CPU
    if (const auto now = std::chrono::steady_clock::now();
        prev_frame_time > now - min_frame_interval) [[unlikely]] {
            Log.debug("Sleeping for framerate limit");
            std::this_thread::sleep_for(min_frame_interval - (now - prev_frame_time));
        }

    cv::Mat image = ctrler()->get_image();
    prev_frame_time = std::chrono::steady_clock::now();

    Matcher matcher(image);
    matcher.set_task_info("Tales@RA@PS-InBattle");
    if (!matcher.analyze()) {
        Log.info(__FUNCTION__, "| 已不在战斗中");
        return false;
    }
    if (!update_cost(image)) {
        m_cost = 0;
    }

    switch (m_strategy) {
    case 0: {
        // ———————— 虫影密植 通用 ————————
        switch (m_step) {
        case 0:
            if (m_cost < 19) {
                return true;
            }
            // clp_pd_callback("阿斯卡伦");
            deploy_oper(840, 660, 832, 660, 430, 530, battle::DeployDirection::Right, true, 580, 405);
            // 夜刀
            deploy_oper(120, 660, 177, 660, 500, 425, battle::DeployDirection::Right, true, 615, 360);
            retreat_oper(535, 355, 605, 230);
            break;
        case 1:
            if (m_cost < 5) {
                // 夜刀
                deploy_oper(120, 660, 177, 660, 625, 375, battle::DeployDirection::Right);
                retreat_oper(535, 355, 605, 230);
                return true;
            }
            // 3级源石虫引诱器械;
            deploy_oper(920, 660, 904, 660, 775, 290, battle::DeployDirection::None);
            break;
        case 2:
            if (m_cost < 5) {
                // 夜刀
                deploy_oper(120, 660, 177, 660, 625, 375, battle::DeployDirection::Right);
                retreat_oper(535, 355, 605, 230);
                return true;
            }
            // 2级源石虫引诱器械;
            deploy_oper(1000, 660, 977, 660, 780, 350, battle::DeployDirection::None);
            break;
        case 3:
            if (m_cost < 5) {
                // 夜刀
                deploy_oper(120, 660, 177, 660, 625, 375, battle::DeployDirection::Right);
                retreat_oper(535, 355, 605, 230);
                return true;
            }
            // 1级源石虫引诱器械;
            deploy_oper(1080, 660, 1050, 660, 700, 300, battle::DeployDirection::None);
            speed_up();
            break;
        case 4: {
            if (m_cost < 40) {
                Matcher slugAnalyzer(image);
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 625, 375, battle::DeployDirection::Left);
                    retreat_oper(535, 355, 605, 230);
                }
                else {
                    slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up");
                    if (!slugAnalyzer.analyze()) {
                        // 夜刀
                        deploy_oper(120, 660, 177, 660, 695, 235, battle::DeployDirection::Up);
                        retreat_oper(625, 230, 605, 230);
                    }
                }
                return true;
            }
            // 3级源石虫引诱器械;
            retreat_oper(625, 290, 605, 230);
            deploy_oper(920, 660, 904, 660, 700, 300, battle::DeployDirection::None);
            break;
        }
        case 5: {
            if (m_cost < 5) {
                Matcher slugAnalyzer(image);
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 625, 375, battle::DeployDirection::Left);
                    retreat_oper(535, 355, 605, 230);
                }
                else {
                    slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up");
                    if (!slugAnalyzer.analyze()) {
                        // 夜刀
                        deploy_oper(120, 660, 177, 660, 695, 235, battle::DeployDirection::Up);
                        retreat_oper(625, 230, 605, 230);
                    }
                }
                return true;
            }
            // 2级源石虫引诱器械;
            deploy_oper(1000, 660, 977, 660, 865, 350, battle::DeployDirection::None);
            break;
        }
        default: {
            Matcher slugAnalyzer(image);
            slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left");
            if (!slugAnalyzer.analyze()) {
                // 夜刀
                deploy_oper(120, 660, 177, 660, 625, 375, battle::DeployDirection::Left);
                retreat_oper(535, 355, 605, 230);
            }
            else {
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 695, 235, battle::DeployDirection::Up);
                    retreat_oper(625, 230, 605, 230);
                }
            }
            return true;
        }
        } // switch m_step
        break;
    } // switch m_strategy case 0
    case 1: {
        // ———————— 虫影密植 右下角 ————————
        switch (m_step) {
        case 0:
            if (m_cost < 18) {
                return true;
            }
            // 滑动到指地点
            ctrler()->swipe(Point(640, 570), Point(700, 0), 500, false, 3, 0);
            sleep(1000);
            // 四月
            deploy_oper(280, 660, 323, 660, 625, 460, battle::DeployDirection::Right);
            sleep(4800); // sleep(3200);
            retreat_oper(540, 450, 565, 280);
            // 四月
            deploy_oper(360, 660, 323, 660, 670, 375, battle::DeployDirection::Right);
            sleep(1000);
            // 离开当前区块
            ProcessTask(*this, {"Tales@RA@LeaveCurrentZone"}).run();
            sleep(1000);
            ProcessTask(*this, {"Tales@RA@DialogConfirmYellow"}).run();
            sleep(1000);
            break;
        default:
            break;
        } // switch m_step
        break;
    } // switch m_strategy case 1
    case 2: {
        // ———————— 幽邃巨洞 中央 ————————
        switch (m_step) {
        case 0:
            if (m_cost < 19) {
                return true;
            }
            // clp_pd_callback("阿斯卡伦");
            deploy_oper(840, 660, 832, 660, 810, 110, battle::DeployDirection::Right, true, 755, 205);
            speed_up();
            break;
        case 1: {
            Matcher slugAnalyzer(image);
            slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Middle");
            if (!slugAnalyzer.analyze()) {
                // 夜刀
                deploy_oper(120, 660, 177, 660, 765, 280, battle::DeployDirection::Left);
                retreat_oper(700, 280, 605, 230);
            }
            else if (m_cost >= 99) {
                // 离开当前区块
                ProcessTask(*this, {"Tales@RA@LeaveCurrentZone"}).run();
                sleep(1000);
                ProcessTask(*this, {"Tales@RA@DialogConfirmYellow"}).run();
                sleep(1000);
            }
            return true;
        }
        default:
            break;
        } // switch m_step
        break;
    } // switch m_strategy case 2
    case 3: {
        // ———————— 幽邃巨洞 左上 ————————
        switch (m_step) {
        case 0: {
            if (m_cost < 7) {
                return true;
            }
            // 滑动到指地点
            ctrler()->swipe(Point(640, 0), Point(800, 720), 500, false, 3, 0);
            sleep(1000);
            // 缄默德克萨斯
            deploy_oper(200, 660, 250, 660, 555, 170, battle::DeployDirection::Left, true, 630, 140);
            sleep(2000);
            retreat_oper(565, 130, 590, 160);
            break;
        }
        case 1: {
            if (m_cost < 19) {
                return true;
            }
            // clp_pd_callback("阿斯卡伦");
            deploy_oper(840, 660, 832, 660, 510, 545, battle::DeployDirection::Down, true, 620, 405);
            sleep(500);
            break;
        }
        case 2: {
            if (m_cost < 5) {
                return true;
            }
            // 3级源石虫引诱器械;
            deploy_oper(920, 660, 904, 660, 545, 430, battle::DeployDirection::None);
            sleep(1000);
            // 夜刀
            deploy_oper(120, 660, 177, 660, 560, 375, battle::DeployDirection::Up);
            retreat_oper(465, 365, 535, 250);
            // 夜刀
            deploy_oper(120, 660, 177, 660, 640, 350, battle::DeployDirection::Up);
            retreat_oper(560, 345, 570, 240);
            break;
        }
        case 3: {
            if (m_cost < 5) {
                return true;
            }
            // 2级源石虫引诱器械;
            deploy_oper(1000, 660, 977, 660, 720, 415, battle::DeployDirection::None);
            // 夜刀
            deploy_oper(120, 660, 177, 660, 675, 335, battle::DeployDirection::Up);
            retreat_oper(600, 335, 570, 240);
            break;
        }
        case 4: {
            if (m_cost < 5) {
                return true;
            }
            // 1级源石虫引诱器械;
            deploy_oper(1080, 660, 1050, 660, 850, 395, battle::DeployDirection::None);
            break;
        }
        case 5: {
            if (m_cost < 5) {
                Matcher slugAnalyzer(image);
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up2");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 830, 250, battle::DeployDirection::Up);
                    retreat_oper(770, 255, 605, 230);
                }
                else {
                    slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left2");
                    if (!slugAnalyzer.analyze()) {
                        // 夜刀
                        deploy_oper(120, 660, 177, 660, 675, 335, battle::DeployDirection::Up);
                        retreat_oper(600, 335, 570, 240);
                    }
                }
                return true;
            }
            speed_up();
            break;
        }
        case 6: {
            if (m_cost < 40) {
                Matcher slugAnalyzer(image);
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up2");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 830, 250, battle::DeployDirection::Up);
                    retreat_oper(770, 255, 605, 230);
                }
                else {
                    slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left2");
                    if (!slugAnalyzer.analyze()) {
                        // 夜刀
                        deploy_oper(120, 660, 177, 660, 675, 335, battle::DeployDirection::Up);
                        retreat_oper(600, 335, 570, 240);
                    }
                }
                return true;
            }
            // 3级源石虫引诱器械;
            retreat_oper(785, 390, 605, 230);
            deploy_oper(920, 660, 904, 660, 850, 395, battle::DeployDirection::None);
            break;
        }
        case 7: {
            Matcher slugAnalyzer(image);
            slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up2");
            if (!slugAnalyzer.analyze()) {
                // 夜刀
                deploy_oper(120, 660, 177, 660, 830, 250, battle::DeployDirection::Up);
                retreat_oper(770, 255, 605, 230);
            }
            else {
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left2");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 675, 335, battle::DeployDirection::Up);
                    retreat_oper(600, 335, 570, 240);
                }
            }
            return true;
        }
        default:
            break;
        } // switch m_step
        break;
    } // switch m_strategy case 3
    case 4: {
        // ———————— 幽邃巨洞 左上 ————————
        switch (m_step) {
        case 0: {
            if (m_cost < 17) {
                return true;
            }
            // 滑动到指地点
            ctrler()->swipe(Point(150, 150), Point(150, 720), 500, false, 3, 0);
            sleep(500);
            ctrler()->swipe(Point(150, 150), Point(1280, 720), 500, false, 3, 0);
            sleep(500);
            // 夜刀
            deploy_oper(280, 660, 323, 660, 665, 155, battle::DeployDirection::Left, true, 665, 155);
            sleep(2000);
            retreat_oper(600, 132, 590, 155);
            sleep(500);
            // 缄默德克萨斯
            deploy_oper(120, 660, 178, 660, 440, 550, battle::DeployDirection::Down, true, 590, 410);
            sleep(500);
            // 夜刀
            deploy_oper(40, 660, 59, 660, 515, 425, battle::DeployDirection::Up, true, 615, 360);
            retreat_oper(540, 365, 565, 250);
            // 夜刀
            deploy_oper(40, 660, 59, 660, 670, 345, battle::DeployDirection::Up, false);
            retreat_oper(595, 340, 570, 240);
            // 夜刀
            deploy_oper(40, 660, 59, 660, 680, 340, battle::DeployDirection::Up, false);
            retreat_oper(600, 335, 570, 240);
            break;
        }
        case 1: {
            if (m_cost < 35) {
                // 夜刀
                deploy_oper(40, 660, 59, 660, 680, 340, battle::DeployDirection::Up, false);
                retreat_oper(600, 335, 570, 240);
                return true;
            }
            // 离开当前区块
            ProcessTask(*this, {"Tales@RA@LeaveCurrentZone"}).run();
            sleep(1000);
            ProcessTask(*this, {"Tales@RA@DialogConfirmYellow"}).run();
            sleep(1000);
            break;
        }
        case 2: {
            if (m_cost < 5) {
                return true;
            }
            // 3级源石虫引诱器械;
            deploy_oper(920, 660, 904, 660, 545, 430, battle::DeployDirection::None);
            sleep(1000);
            // 夜刀
            deploy_oper(120, 660, 177, 660, 560, 375, battle::DeployDirection::Up);
            retreat_oper(465, 365, 535, 250);
            // 夜刀
            deploy_oper(120, 660, 177, 660, 640, 350, battle::DeployDirection::Up);
            retreat_oper(560, 345, 570, 240);
            break;
        }
        case 3: {
            if (m_cost < 5) {
                return true;
            }
            // 2级源石虫引诱器械;
            deploy_oper(1000, 660, 977, 660, 720, 415, battle::DeployDirection::None);
            // 夜刀
            deploy_oper(120, 660, 177, 660, 675, 335, battle::DeployDirection::Up);
            retreat_oper(600, 335, 570, 240);
            break;
        }
        case 4: {
            if (m_cost < 5) {
                return true;
            }
            // 1级源石虫引诱器械;
            deploy_oper(1080, 660, 1050, 660, 850, 395, battle::DeployDirection::None);
            break;
        }
        case 5: {
            if (m_cost < 5) {
                Matcher slugAnalyzer(image);
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up2");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 830, 250, battle::DeployDirection::Up);
                    retreat_oper(770, 255, 605, 230);
                }
                else {
                    slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left2");
                    if (!slugAnalyzer.analyze()) {
                        // 夜刀
                        deploy_oper(120, 660, 177, 660, 675, 335, battle::DeployDirection::Up);
                        retreat_oper(600, 335, 570, 240);
                    }
                }
                return true;
            }
            speed_up();
            break;
        }
        case 6: {
            if (m_cost < 40) {
                Matcher slugAnalyzer(image);
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up2");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 830, 250, battle::DeployDirection::Up);
                    retreat_oper(770, 255, 605, 230);
                }
                else {
                    slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left2");
                    if (!slugAnalyzer.analyze()) {
                        // 夜刀
                        deploy_oper(120, 660, 177, 660, 675, 335, battle::DeployDirection::Up);
                        retreat_oper(600, 335, 570, 240);
                    }
                }
                return true;
            }
            // 3级源石虫引诱器械;
            retreat_oper(785, 390, 605, 230);
            deploy_oper(920, 660, 904, 660, 850, 395, battle::DeployDirection::None);
            break;
        }
        case 7: {
            Matcher slugAnalyzer(image);
            slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Up2");
            if (!slugAnalyzer.analyze()) {
                // 夜刀
                deploy_oper(120, 660, 177, 660, 830, 250, battle::DeployDirection::Up);
                retreat_oper(770, 255, 605, 230);
            }
            else {
                slugAnalyzer.set_task_info("Tales@RA@PS-NoSlug_Left2");
                if (!slugAnalyzer.analyze()) {
                    // 夜刀
                    deploy_oper(120, 660, 177, 660, 675, 335, battle::DeployDirection::Up);
                    retreat_oper(600, 335, 570, 240);
                }
            }
            return true;
        }
        default:
            break;
        } // switch m_step
        break;
    } // switch m_strategy case 4
    case 5: {
        // ———————— 幽邃巨洞 左上 ————————
        switch (m_step) {
        case 0: {
            if (m_cost < 17) {
                return true;
            }
            // 滑动到指地点
            ctrler()->swipe(Point(150, 360), Point(1280, 360), 500, false, 3, 0);
            sleep(500);
            // 伊内斯
            deploy_oper(360, 660, 396, 660, 595, 275, battle::DeployDirection::Right);
            sleep(1000);
            break;
        }
        case 1: {
            if (m_cost < 19) {
                return true;
            }
            // 阿斯卡伦
            deploy_oper(760, 660, 759, 660, 595, 340, battle::DeployDirection::Down);
            sleep(1000);
            // 夜刀
            deploy_oper(43, 660, 59, 660, 670, 260, battle::DeployDirection::Down);
            retreat_oper(595, 250, 565, 205);
            // 夜刀
            deploy_oper(43, 660, 59, 660, 665, 300, battle::DeployDirection::Down);
            retreat_oper(590, 290, 560, 220);
            speed_up();
            break;
        }
        case 2: {
            if (m_cost < 30) {
                return true;
            }
            else if (m_cost < 40) {
                // 夜刀
                deploy_oper(43, 660, 59, 660, 665, 330, battle::DeployDirection::Down);
                retreat_oper(590, 310, 560, 230);
                return true;
            }
            else {
                // 离开当前区块
                ProcessTask(*this, {"Tales@RA@LeaveCurrentZone"}).run();
                sleep(1000);
                ProcessTask(*this, {"Tales@RA@DialogConfirmYellow"}).run();
                sleep(1000);
                return true;
            }
        }
        default:
            break;
        } // switch m_step
        break;
    } // switch m_strategy case 5
    default:
        break;
    } // switch m_strategy

    m_first_deploy = false;
    ++m_step;

    return true;
}

bool asst::ReclamationBattleTaskPlugin::deploy_oper(
    int x,
    int y,
    int oper_x,
    int oper_y,
    int target_x,
    int target_y,
    battle::DeployDirection direction,
    bool camera_move,
    int direct_x,
    int direct_y)
{
    const auto swipe_oper_task_ptr = Task.get("BattleSwipeOper");
    const auto use_oper_task_ptr = Task.get("BattleUseOper");

    Point target_point(target_x, target_y);

    int dist = static_cast<int>(Point::distance(target_point, { oper_x, oper_y }));

    // 1000 是随便取的一个系数，把整数的 pre_delay 转成小数用的
    int duration = static_cast<int>(dist / 1000.0 * swipe_oper_task_ptr->pre_delay) + 200;
    // 时间太短了的压根放不上去，故意加长一点
    if (int min_duration = swipe_oper_task_ptr->special_params.at(4) + 200; duration < min_duration) {
        duration = min_duration;
    }
    bool deploy_with_pause = ControlFeat::support(ctrler()->support_features(), ControlFeat::SWIPE_WITH_PAUSE);

    Point click_point(x, y);
    ctrler()->click(click_point);
    sleep(use_oper_task_ptr->pre_delay + 100);

    Point oper_point(oper_x, oper_y);
    ctrler()->swipe(
        oper_point,
        target_point,
        duration,
        false,
        swipe_oper_task_ptr->special_params.at(2),
        swipe_oper_task_ptr->special_params.at(3),
        deploy_with_pause);

    sleep(use_oper_task_ptr->post_delay);

    if (camera_move) {
        sleep(500);
        target_point.x = direct_x;
        target_point.y = direct_y;
    }

    // 拖动干员朝向
    if (direction != battle::DeployDirection::None) {
        static const std::unordered_map<battle::DeployDirection, Point> DirectionMap = {
            { battle::DeployDirection::Right, Point::right() }, { battle::DeployDirection::Down, Point::down() },
            { battle::DeployDirection::Left, Point::left() },   { battle::DeployDirection::Up, Point::up() },
            { battle::DeployDirection::None, Point::zero() },
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
        sleep(use_oper_task_ptr->pre_delay);
    }

    return true;
}

bool asst::ReclamationBattleTaskPlugin::retreat_oper(
    const int& oper_x,
    const int& oper_y,
    const int& retreat_x,
    const int& retreat_y,
    const bool& camera_move)
{
    const auto use_oper_task_ptr = Task.get("BattleUseOper");

    const Point oper_point(oper_x, oper_y);
    ctrler()->click(oper_point);

    sleep(use_oper_task_ptr->pre_delay + 100);
    if (camera_move) {
        sleep(500);
    }

    const Point retreat_point(retreat_x, retreat_y);
    ctrler()->click(retreat_point);
    // 仅简单复用，该延迟含义与此处逻辑无关 by MistEO
    sleep(use_oper_task_ptr->pre_delay);
    
    return true;
}
