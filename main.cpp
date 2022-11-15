#include <cpp-httplib/httplib.h>
#include <spdlog/spdlog.h>
#include "elec_hole.h"

std::vector<ohtoai::LayoutSolution> getPathSolution(const ohtoai::MapInfo& map, const std::string& id);
std::map<std::string, ohtoai::MapInfo> MapSet;

// 存储MapSet到map.json
void saveMapSet() {
	std::ofstream ofs("map.json");
	if (!ofs)
	{
		spdlog::error("Cannot write to map.json");
	}
	try
	{
		ofs << nlohmann::json(MapSet);
	}
	catch (const std::exception& e)
	{
		spdlog::error("Cannot write to map.json: {}", e.what());
	}
	ofs.close();
}

// 从map.json读取MapSet，处理异常nlohmann::json的异常
void loadMapSet() {
	std::ifstream ifs("map.json");
	if (!ifs.is_open()) {
		return;
	}
	nlohmann::json j;
	try {
		ifs >> j;
	}
	catch (std::exception& e) {
		spdlog::error("Cannot read map.json: {}", e.what());
		return;
	}
	for (auto& [id, map] : j.items()) {
		MapSet[id] = map;
	}
}


int main(int argc, char** argv)
{
	using namespace::ohtoai;
	using namespace nlohmann::literals;
	using namespace httplib;

	Server svr;

	loadMapSet();

	svr.set_logger([](const Request& req, const Response& res) {
		spdlog::info("{} {} {} {} {} {}", req.remote_addr, req.method, req.path, res.status, req.get_header_value("User-Agent"), req.body);
		});

	svr.Get("/api/map", [&](const Request& req, Response& res)
		{
			try
	{
		auto map = MapSet.at(req.get_param_value("map"));
		res.set_content(nlohmann::json(map).dump(4), "application/json");
	}
	catch (const std::out_of_range&e)
	{
		res.status = 404;
		nlohmann::json ret_body;
		ret_body["status"] = "error";
		ret_body["message"] = e.what();
		res.set_content(ret_body.dump(4), "application/json");
		spdlog::error("{}", e.what());
	}
	catch (const std::exception& e)
	{
		res.status = 406;
		nlohmann::json ret_body;
		ret_body["status"] = "error";
		ret_body["message"] = e.what();
		res.set_content(ret_body.dump(4), "application/json");
		spdlog::error("{}", e.what());
	}
		});

	svr.Post("/api/map", [&](const Request& req, Response& res)
		{
			try
	{
		auto map = nlohmann::json::parse(req.body);
		MapSet[req.get_param_value("map")] = map.get<MapInfo>();
		saveMapSet();
		res.status = 201;
		nlohmann::json ret_body;
		ret_body["status"] = "ok";
		res.set_content(ret_body.dump(4), "application/json");
	}
	catch (const std::exception& e)
	{
		res.status = 406;
		nlohmann::json ret_body;
		ret_body["status"] = "error";
		ret_body["message"] = e.what();
		res.set_content(ret_body.dump(4), "application/json");
		spdlog::error("{}", e.what());
	}
		});

	svr.Get("/api/map_demo", [&](const Request& req, Response& res)
		{
			try
	{
		auto map = R"(
{
    "house_groups": [
        {
            "house_poles": [
                {
                    "x": 95,
                    "y": 6,
                    "id": "#864998"
                },
                {
                    "x": 45,
                    "y": 2,
                    "id": "#26751"
                },
                {
                    "x": 55,
                    "y": 64,
                    "id": "#25236"
                },
                {
                    "x": 89,
                    "y": 98,
                    "id": "#09"
                },
                {
                    "x": 77,
                    "y": 60,
                    "id": "#7"
                }
            ],
            "group_front_pole": {
                "x": 44,
                "y": 64,
                "id": "#27"
            },
            "group_back_pole": {
                "x": 70,
                "y": 67,
                "id": "#81597"
            },
            "group_back_valid": true,
            "group_front_valid": true
        },
        {
            "house_poles": [
                {
                    "x": 62,
                    "y": 45,
                    "id": "#42568"
                }
            ],
            "group_front_pole": {
                "x": 21,
                "y": 16,
                "id": "#718937"
            },
            "group_back_pole": {
                "x": 77,
                "y": 70,
                "id": "#335982"
            },
            "group_back_valid": true,
            "group_front_valid": true
        },
        {
            "house_poles": [
                {
                    "x": 91,
                    "y": 61,
                    "id": "#975"
                }
            ],
            "group_front_pole": {
                "x": 71,
                "y": 98,
                "id": "#23791"
            },
            "group_back_pole": {
                "x": 94,
                "y": 92,
                "id": "#941441"
            },
            "group_back_valid": true,
            "group_front_valid": true
        }
    ],
    "elec_poles": [
        {
            "x": 92,
            "y": 78,
            "id": "#2836"
        },
        {
            "x": 60,
            "y": 84,
            "id": "#425384"
        },
        {
            "x": 61,
            "y": 30,
            "id": "#591"
        }
    ]
}
)"_json;
		res.set_content(map.dump(4), "application/json");
	}
	catch (const std::out_of_range&e)
	{
		res.status = 404;
		nlohmann::json ret_body;
		ret_body["status"] = "error";
		ret_body["message"] = e.what();
		res.set_content(ret_body.dump(4), "application/json");
		spdlog::error("{}", e.what());
	}
	catch (const std::exception& e)
	{
		res.status = 406;
		nlohmann::json ret_body;
		ret_body["status"] = "error";
		ret_body["message"] = e.what();
		res.set_content(ret_body.dump(4), "application/json");
		spdlog::error("{}", e.what());
	}
		});

	svr.Get("/api/solution", [&](const Request& req, Response& res)
		{
			try
	{
		auto map = MapSet.at(req.get_param_value("map"));
		auto solution = getPathSolution(map, req.get_param_value("house"));
		nlohmann::json data;
		// 遍历输出slns
		for (auto& sln : solution)
		{
			nlohmann::json sln_j = sln;
			std::cout << sln_j.dump(4) << std::endl << std::endl;

			nlohmann::json j;
			for (auto hp : sln.path)
			{
				nlohmann::json p = hp;
				p["type"] = "house";
				j.push_back(p);
			}
			nlohmann::json edpt = sln.house_endpoint_pole;
			edpt["type"] = "endpoint";
			j.push_back(edpt);
			nlohmann::json elec = sln.elec_pole;
			elec["type"] = "elec";
			j.push_back(elec);
			data.push_back(j);
		}
		res.set_content(data.dump(4), "application/json");
	}
	catch (const std::out_of_range&e)
	{
		res.status = 404;
		nlohmann::json ret_body;
		ret_body["status"] = "error";
		ret_body["message"] = e.what();
		res.set_content(ret_body.dump(4), "application/json");
		spdlog::error("{}", e.what());
	}
	catch (const std::exception& e)
	{
		res.status = 406;
		nlohmann::json ret_body;
		ret_body["status"] = "error";
		ret_body["message"] = e.what();
		res.set_content(ret_body.dump(4), "application/json");
		spdlog::error("{}", e.what());
	}
		});

	int port{};
	// read port from argv
	if (argc > 1)
	{
		port = std::stoi(argv[1]);
	}
	else
	{
		port = 8099;
	}
	spdlog::info("server listening on port {}", port);
	svr.listen("localhost", port);
	return 0;
}

namespace ohtoai
{
	double distance(const Hole& h1, const Hole& h2)
	{
		return std::sqrt(std::pow(h1.x - h2.x, 2) + std::pow(h1.y - h2.y, 2));
	}
}

std::vector<ohtoai::LayoutSolution> getPathSolution(const ohtoai::MapInfo& map, const std::string& house_hole_id)
{
	std::vector<ohtoai::LayoutSolution> solutions{};

	const auto& [hg, hp] = [&map, &house_hole_id] {
		// 遍历map的house group
		for (const auto& hg : map.house_groups)
		{
			// 遍历house group的house pole
			for (const auto& hp : hg.house_poles)
			{
				if (hp.id == house_hole_id)
				{
					return std::tuple{ hg , hp };
				}
			}
		}
		throw "no such house hole id: " + house_hole_id;
	}();
	auto& house_group = hg;
	auto& house_pole = hp;

	auto front_valid = house_group.group_front_valid;
	auto back_valid = house_group.group_back_valid;

	auto house_index = [&house_group, &house_pole] {
		for (size_t i = 0; i < house_group.house_poles.size(); ++i)
		{
			if (house_group.house_poles[i].id == house_pole.id)
			{
				return i;
			}
		}
		throw "no such house hole id: " + house_pole.id;
	}();


	const auto [front_distance, back_distance] = [&house_group, &house_pole, house_index] {
		// 计算front 到index的距离之和
		auto front_distance = 0.0;
		for (size_t i = 0; i < house_index; ++i)
		{
			front_distance += ohtoai::distance(house_group.house_poles[i], house_group.house_poles[i + 1]);
		}
		// 计算back 到index的距离之和
		auto back_distance = 0.0;
		for (size_t i = house_index; i < house_group.house_poles.size() - 1; ++i)
		{
			back_distance += ohtoai::distance(house_group.house_poles[i], house_group.house_poles[i + 1]);
		}
		return std::tuple{ front_distance, back_distance };
	}();

	auto& front_nearest_elec_hole = *std::min_element(map.elec_poles.begin(), map.elec_poles.end(), [&house_group](const auto& e1, const auto& e2) {
		return ohtoai::distance(e1, house_group.group_front_pole) < ohtoai::distance(e2, house_group.group_front_pole);
		});

	auto& back_nearest_elec_hole = *std::min_element(map.elec_poles.begin(), map.elec_poles.end(), [&house_group](const auto& e1, const auto& e2) {
		return ohtoai::distance(e1, house_group.group_back_pole) < ohtoai::distance(e2, house_group.group_back_pole);
		});


	if (front_valid && back_valid && front_nearest_elec_hole.id == back_nearest_elec_hole.id)
	{
		if (ohtoai::distance(front_nearest_elec_hole, house_group.group_front_pole)
			< ohtoai::distance(back_nearest_elec_hole, house_group.group_back_pole))
		{
			back_valid = false;
		}
		else
		{
			front_valid = false;
		}
	}

	if (front_valid)
	{
		ohtoai::LayoutSolution sln{};
		for (int i = house_index; i >= 0; --i)
		{
			sln.path.push_back(house_group.house_poles[i]);
		}
		sln.house_endpoint_pole = house_group.group_front_pole;
		sln.elec_pole = front_nearest_elec_hole;
		sln.distance = ohtoai::distance(front_nearest_elec_hole, house_group.group_front_pole) + back_distance;
		solutions.push_back(sln);
	}

	if (back_valid)
	{
		ohtoai::LayoutSolution sln{};
		for (int i = house_index; i < house_group.house_poles.size(); ++i)
		{
			sln.path.push_back(house_group.house_poles[i]);
		}
		sln.house_endpoint_pole = house_group.group_back_pole;
		sln.elec_pole = back_nearest_elec_hole;
		sln.distance = ohtoai::distance(back_nearest_elec_hole, house_group.group_back_pole) + front_distance;
		solutions.push_back(sln);
	}

	return solutions;
}
