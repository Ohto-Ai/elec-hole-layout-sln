//  To parse this JSON data, first install
//
//      Boost     http://www.boost.org
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     ohtoai::MapInfo data = nlohmann::json::parse(jsonString);

#pragma once

#include <nlohmann/json.hpp>

namespace ohtoai {
    using nlohmann::json;

    /**
     * Hole，坐标
     *
     * 组后节点
     *
     * 组前结点
     */
    struct Hole {
        /**
         * 索引ID或index
         */
        std::string id;
        double x;
        double y;
        nlohmann::json extra;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Hole, id, x, y, extra);
    };

    /**
     * HouseGroup，房屋组数据
     */
    struct HouseGroup {
        /**
         * 组后节点
         */
        ohtoai::Hole group_back_pole;
        bool group_back_valid;
        /**
         * 组前结点
         */
        ohtoai::Hole group_front_pole;
        bool group_front_valid;
        /**
         * 住户结点
         */
        std::vector<ohtoai::Hole> house_poles;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(HouseGroup, group_back_pole, group_front_pole, group_back_valid, group_front_valid, house_poles);
    };

    /**
     * MapInfo
     */
    struct MapInfo {
        /**
         * 电线杆信息
         */
        std::vector<ohtoai::Hole> elec_poles;
        /**
         * 所有房屋组信息
         */
        std::vector<ohtoai::HouseGroup> house_groups;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(MapInfo, elec_poles, house_groups);
    };

    /**
     * LayoutSolution
     */
    struct LayoutSolution {
        double distance;
        std::vector<Hole> path;
		Hole house_endpoint_pole;
		Hole elec_pole;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(LayoutSolution, distance, path, house_endpoint_pole, elec_pole);
    };
}
