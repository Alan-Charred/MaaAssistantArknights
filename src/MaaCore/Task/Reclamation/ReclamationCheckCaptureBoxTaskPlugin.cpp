#include "ReclamationCheckCaptureBoxTaskPlugin.h"

#include <algorithm>

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/Matcher.h"
#include "Vision/RegionOCRer.h"

bool asst::ReclamationCheckCaptureBoxTaskPlugin::load_params([[maybe_unused]] const json::value& params)
{
    LogTraceFunction;

    if (const ReclamationMode& mode = m_config->get_mode(); mode != ReclamationMode::PerfectSlugs) {
        return false;
    }

    return true;
}

bool asst::ReclamationCheckCaptureBoxTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    const std::string& task_name = details.get("details", "task", "");
    if (task_name == m_config->get_theme() + "@RA@PS-CheckCaptureBox") {
        return true;
    }

    return false;
}

bool asst::ReclamationCheckCaptureBoxTaskPlugin::_run()
{
    LogTraceFunction;

    std::optional<int> capacity = analyze_capacity();
    custom_callback("检测到 " + (capacity.has_value() ? std::to_string(capacity.value()) : "?") + " 只源石虫");

    for (int i = 0; i < (capacity.has_value() ? capacity.value() : 100); ++i) {
        Point p(
            m_item_origin.x + i % 5 * m_item_horizontal_offset,
            m_item_origin.y + std::min(i / 5, 2) * m_item_vertical_offset);
        ctrler()->click(p);
        sleep(200);

        const std::string slug_name = get_slug_name();
        const unsigned int stars = count_stars();
        const unsigned int max_stats = count_max_stats();
        custom_callback(
            "#" + std::to_string(i + 1) + ": " + std::to_string(stars) + " 星 " + std::to_string(max_stats) +
            " MAX 属性 " + slug_name);

        if (stars <= 2) {
            return true;
        }
        if (max_stats >= 5) {
            m_task_ptr->set_enable(false);
            return true;
        }
        if (i % 5 == 4 && i / 5 >= 2) {
            ProcessTask(*this, { "Tales@RA@PS-ScrollDown" }).run();
            sleep(500);
        }
    }

    return true;
}

std::optional<int> asst::ReclamationCheckCaptureBoxTaskPlugin::analyze_capacity() const
{
    LogTraceFunction;

    RegionOCRer capacity_analyzer(ctrler()->get_image());
    capacity_analyzer.set_task_info("Tales@RA@PS-CaptureBoxCapacity");

    auto capacity_opt = capacity_analyzer.analyze();
    if (!capacity_opt) {
        return std::nullopt;
    }
    std::string capacity_str = capacity_opt->text;

    std::string::size_type pos = capacity_str.find_last_not_of("0123456789/");
    if (pos != std::string::npos) {
        capacity_str = capacity_str.substr(pos + 1);
    }
    if (!ranges::all_of(capacity_str, [](const char& c) -> bool { return std::isdigit(c) || c == '/'; }) ||
        !capacity_str.ends_with("/100")) {
        return std::nullopt;
    }
    return std::stoi(capacity_str.substr(0, capacity_str.size() - 4));
}

std::string asst::ReclamationCheckCaptureBoxTaskPlugin::get_slug_name() const
{
    LogTraceFunction;

    RegionOCRer slug_name_analyzer(ctrler()->get_image());
    slug_name_analyzer.set_task_info("Tales@RA@SlugName");

    auto slug_name_opt = slug_name_analyzer.analyze();

    return slug_name_opt ? slug_name_opt->text : "未知源石虫";
}

unsigned int asst::ReclamationCheckCaptureBoxTaskPlugin::count_stars() const
{
    int count = 3;
    Matcher matcher(ctrler()->get_image());

    for (; count > 0; --count) {
        matcher.set_task_info("Tales@RA@PS-Star_" + std::to_string(count));
        if (matcher.analyze()) {
            return count;
        }
    }

    return count;
}

unsigned int asst::ReclamationCheckCaptureBoxTaskPlugin::count_max_stats() const
{
    unsigned int count = 0;
    Matcher matcher(ctrler()->get_image());

    for (int i = 1; i <= 5; ++i) {
        matcher.set_task_info("Tales@RA@PS-MAX_" + std::to_string(i));
        if (matcher.analyze()) {
            count += 1;
        }
    }
    return count;
}

void asst::ReclamationCheckCaptureBoxTaskPlugin::custom_callback(const std::string& msg)
{
    json::value info = basic_info_with_what("CustomCallback");
    info["details"]["message"] = msg;
    callback(AsstMsg::SubTaskExtraInfo, info);
}
