#pragma once

#include "AbstractReclamationAlgorithmTaskPlugin.h"
#include "Common/AsstTypes.h"

namespace asst
{
    class ReclamationAlgorithmPerfectSlugTaskPlugin : public AbstractReclamationAlgorithmTaskPlugin
    {
    public:
        using AbstractReclamationAlgorithmTaskPlugin::AbstractReclamationAlgorithmTaskPlugin;
        virtual ~ReclamationAlgorithmPerfectSlugTaskPlugin() override = default;

        virtual bool verify(AsstMsg msg, const json::value& details) const override;

    protected:
        virtual bool _run() override;

    private:
        const Point m_swipe_begin = {640, 360}; // 滑动起点
        const Point m_swipe_end = {640, 216};   // 滑动终点

        const Point m_click_origin = {93, 175};
        const int m_horizontal_offset = 165;
        const int m_vertical_offset = 207;

        bool check_perfect_slug();
        unsigned int count_stars();
        unsigned int count_max_stats();
        std::optional<int> capacity_analyze() const;
        void clp_pd_callback(std::string cur, int deepen_or_weaken = 0, std::string prev = "");
    };
}
