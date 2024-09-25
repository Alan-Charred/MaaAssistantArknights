#pragma once

#include "AbstractReclamationTaskPlugin.h"

#include "Common/AsstTypes.h"

namespace asst
{
class ReclamationCheckCaptureBoxTaskPlugin : public AbstractReclamationTaskPlugin
{
public:
    using AbstractReclamationTaskPlugin::AbstractReclamationTaskPlugin;
    virtual ~ReclamationCheckCaptureBoxTaskPlugin() override = default;
    virtual bool verify(AsstMsg msg, const json::value& details) const override;
    virtual bool load_params(const json::value& params) override;

protected:
    virtual bool _run() override;

private:
    std::optional<int> analyze_capacity() const;
    std::string get_slug_name() const;
    unsigned int count_stars() const;
    unsigned int count_max_stats() const;
    void custom_callback(const std::string& msg);

    // ———————— constants and variables ———————————————————————————————————————————————
    const Point m_item_origin = { 93, 175 };
    const int m_item_horizontal_offset = 165;
    const int m_item_vertical_offset = 207;
};
} // namespace asst
