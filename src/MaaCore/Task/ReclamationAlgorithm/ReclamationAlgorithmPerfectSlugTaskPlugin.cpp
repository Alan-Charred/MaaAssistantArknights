#include "ReclamationAlgorithmPerfectSlugTaskPlugin.h"

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/OCRer.h"
#include "Vision/RegionOCRer.h"
#include "Vision/Matcher.h"

#include <algorithm>


bool asst::ReclamationAlgorithmPerfectSlugTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

     const auto task_name = details.get("details", "task", "");
     
     if (task_name == "ReclamationAlgorithm@CheckPerfectSlug") {
        return true;
     }

     return false;
}

bool asst::ReclamationAlgorithmPerfectSlugTaskPlugin::_run()
{
    std::optional<int> capacity = capacity_analyze();
    if (capacity) {
        clp_pd_callback("抓捕源石虫: " + std::to_string(capacity.value()));
    }
    check_perfect_slug();
    return true;
}

void asst::ReclamationAlgorithmPerfectSlugTaskPlugin::clp_pd_callback(std::string cur, int deepen_or_weaken, std::string prev)
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

bool asst::ReclamationAlgorithmPerfectSlugTaskPlugin::check_perfect_slug()
{
    for (int i = 0; i < 100; ++i) {
        Point p(m_click_origin.x + i % 5 * m_horizontal_offset, m_click_origin.y + std::min(i / 5, 2) * m_vertical_offset);
        ctrler()->click(p);
        unsigned int stars = count_stars();
        unsigned int max_stats = count_max_stats();
        clp_pd_callback(std::to_string(stars) + "星" + std::to_string(max_stats) + "属性MAX源石虫");
        if (stars <= 2) {
            return true;
        }
        if (max_stats >= 5) {
            m_task_ptr->set_enable(false);
            return true;
        }
        if (i % 5 == 4 && i / 5 >= 2) {
            ctrler()->swipe(m_swipe_begin, m_swipe_end, 500);
            sleep(800);
        }
    }
    return true;
}

unsigned int asst::ReclamationAlgorithmPerfectSlugTaskPlugin::count_stars()
{
    int count = 3;
    Matcher matcher(ctrler()->get_image());
    for (; count > 0; --count) {
        matcher.set_task_info("ReclamationAlgorithm@Star_" + std::to_string(count));
        if (matcher.analyze()) {
            return count;
        }
    }
    return count;
}

unsigned int asst::ReclamationAlgorithmPerfectSlugTaskPlugin::count_max_stats()
{
    unsigned int count = 0;
    Matcher matcher(ctrler()->get_image());
    for (int i = 1; i <= 5; ++i) {
        matcher.set_task_info("ReclamationAlgorithm@MAX_" + std::to_string(i));
        if (matcher.analyze()) {
            count += 1;
        }
    }
    return count;
}

std::optional<int> asst::ReclamationAlgorithmPerfectSlugTaskPlugin::capacity_analyze() const
{
    RegionOCRer capacity_analyzer(ctrler()->get_image());
    capacity_analyzer.set_task_info("ReclamationAlgorithm@CaptureBoxCapacity");
    capacity_analyzer.set_replace(Task.get<OcrTaskInfo>("NumberOcrReplace")->replace_map);

    auto capacity_opt = capacity_analyzer.analyze();
    if (!capacity_opt) {
        return std::nullopt;
    }
    std::string capacity_str = capacity_opt->text;

    std::string::size_type pos  = capacity_str.find_last_not_of("0123456789/");
    if (pos != std::string::npos) {
        capacity_str = capacity_str.substr(pos + 1);
    }
    if (!ranges::all_of(capacity_str, [](const char& c) -> bool { return std::isdigit(c) || c == '/' ; })
        || !capacity_str.ends_with("/100")) {
        return std::nullopt;
    }
    return std::stoi(capacity_str.substr(0, capacity_str.size() - 4));
}
