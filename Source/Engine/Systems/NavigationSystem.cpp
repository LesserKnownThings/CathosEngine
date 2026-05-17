// #include "NavigationSystem.h"
// #include "Components/Navigation/PathRequest.h"
// #include "Debug/DebugSystem.h"
// #include "Map/MapFormat.h"
// #include "Math/FixedMath.hpp"
// #include "Registry/CommandBuffer.h"
// #include "Registry/Registry.h"
// #include "Resources/Navigation/PathNetwork.h"
// #include "Resources/Navigation/PortalMap.h"
// #include "Resources/Navigation/SectorMap.h"
// #include "Systems/SystemRegistry.h"
// #include "Utilities/MapUtils.h"
// #include <algorithm>
// #include <cstdint>
// #include <format>
// #include <glm/fwd.hpp>
// #include <queue>
// #include <vector>

// REGISTER_SYSTEM(NavigationSystem, SystemPhase::Simulation, DEPENDENCIES({}), DEPENDENCIES({}), 0);

// NavigationSystem::NavigationSystem()
// {
//     SystemRegistry& sreg = SystemRegistry::Get();
//     sreg.BindInitFunc<NavigationSystem, &NavigationSystem::Init>(this);
//     sreg.BindSyncFunc<NavigationSystem, &NavigationSystem::RunSync>(this);
//     // }

//     // inline void IndexToHZ(int32_t index, int32_t width, int32_t& x, int32_t& z)
//     // {
//     //     x = index % width;
//     //     z = index / width;
//     // }

//     // inline FixedT Heuristic(const Float3& start, const Float3& goal)
//     // {
//     //     return abs(start.x - goal.x) + abs(start.z - goal.z);
//     // }

//     // struct PortalNode
//     // {
//     //     int32_t portal;
//     //     int32_t parent;

//     //     FixedT gCost;
//     //     FixedT hCost;

//     //     FixedT FCost() const { return gCost + hCost; }
//     // };

//     // struct PQItem
//     // {
//     //     int32_t portal;
//     //     FixedT fCost;

//     //     bool operator>(const PQItem& other) const
//     //     {
//     //         return fCost > other.fCost;
//     //     }
//     // };

//     // inline bool IsHorizontal(const Float3& start, const Float3& end)
//     // {
//     //     const FixedT dx = end.x - start.x;
//     //     const FixedT dz = end.z - start.z;

//     //     return abs(dx) > abs(dz);
//     // }

//     // // Returns a list of portals for the path
//     // inline std::vector<int32_t> AStar(const Float3& start, const Float3& goal, const MapFormat& map, const PortalMap& portalMap)
//     // {
//     //     std::unordered_map<int32_t /*Sector Index*/, PortalNode> nodes;
//     //     std::unordered_map<int32_t, bool> closed;

//     //     std::priority_queue<PQItem, std::vector<PQItem>, std::greater<>> open;

//     //     const int32_t startIndex = map.GetSectorIndex(start);
//     //     auto it = portalMap.portals.find(startIndex);
//     //     if (it == portalMap.portals.end())
//     //         return {};

//     //     const PortalIndices& portalIndices = it->second;

//     //     if (IsHorizontal(start, goal) && portalIndices.horizontal.size() > 0)
//     //     {
//     //         for (int32_t portalIndex : portalIndices.horizontal)
//     //         {
//     //         }
//     //     }
//     //     else
//     //     {
//     //     }

//     //     nodes[startIndex] = { start, 0, Heuristic(start, goal), -1 };
//     //     open.push({ start, nodes[start].FCost() });

//     //     while (!open.empty())
//     //     {
//     //         int32_t current = open.top().sector;
//     //         open.pop();

//     //         if (current == goal)
//     //         {
//     //             std::vector<int32_t> path;
//     //             int32_t s = goal;

//     //             while (s != -1)
//     //             {
//     //                 path.push_back(s);
//     //                 s = nodes[s].parent;
//     //             }

//     //             std::reverse(path.begin(), path.end());
//     //             return path;
//     //         }

//     //         if (closed[current])
//     //             continue;

//     //         closed[current] = true;

//     //         auto it = portalMap.portals.find(current);
//     //         if (it == portalMap.portals.end())
//     //             continue;

//     //         const PortalIndices& indices = it->second;
//     //         std::vector<int32_t> neighbors{};
//     //         neighbors.insert(neighbors.end(), indices.horizontal.begin(), indices.horizontal.end());
//     //         neighbors.insert(neighbors.end(), indices.vertical.begin(), indices.vertical.end());

//     //         for (int32_t i = 0; i < neighbors.size(); ++i)
//     //         {
//     //             int32_t neighbor = neighbors[i];
//     //             if (neighbor < 0)
//     //                 continue;

//     //             int32_t tentativeG = nodes[current].gCost + 1;

//     //             if (!nodes.count(neighbor) || tentativeG < nodes[neighbor].gCost)
//     //             {
//     //                 nodes[neighbor] = {
//     //                     neighbor,
//     //                     tentativeG,
//     //                     Heuristic(neighbor, goal, map),
//     //                     current
//     //                 };

//     //                 open.push({ neighbor, nodes[neighbor].FCost() });
//     //             }
//     //         }
//     //     }

//     //     return {};
//     // }

//     void NavigationSystem::Init(Registry * registry, CommandBuffer & cmd)
//     {
//         MapFormat map{};

//         // TODO add resource instead of hardcoded string
//         // Users will need to be able to select the map they want so I need a resource for this
//         if (MapUtils::ImportMap("Data/Maps/TestMap.cmf", map))
//         {
//             registry->AddResource(std::move(map));
//             registry->AddResource<PathNetworkMap>();
//             SectorMap& sectorMap = registry->AddResource<SectorMap>();
//             PortalMap& portalMap = registry->AddResource<PortalMap>();

//             sectorMap.sectors.reserve(map.sectors.size());
//             for (const SectorData& sector : map.sectors)
//             {
//                 sectorMap.sectors.push_back(Sector{});
//             }

//             for (int32_t i = 0; i < map.portals.size(); ++i)
//             {
//                 const Portal& portal = map.portals[i];

//                 auto it = portalMap.portals.find(portal.sector);
//                 if (it == portalMap.portals.end())
//                 {
//                     PortalIndices indices{};

//                     if (portal.direction == PortalDirection::Horizontal)
//                     {
//                         indices.horizontal.push_back(portal.neighbor);
//                     }
//                     else
//                     {
//                         indices.vertical.push_back(portal.neighbor);
//                     }

//                     portalMap.portals.emplace(portal.sector, indices);
//                 }
//                 else
//                 {
//                     if (portal.direction == PortalDirection::Horizontal)
//                     {
//                         it->second.horizontal.push_back(portal.neighbor);
//                     }
//                     else
//                     {
//                         it->second.vertical.push_back(portal.neighbor);
//                     }
//                 }
//             }
//         }

//         auto entity = cmd.CreateEntity();
//         cmd.AddComponent(entity, PathRequest{ Float3{ 0.0f }, Float3{ 100.0f, 0.0f, 200.0f } });
//     }

//     // bool IsSectorIndexValid(int32_t index, const MapFormat& map)
//     // {
//     //     return index >= 0 && index < map.HorizontalSectors() * map.VerticalSectors();
//     // }

//     // inline int32_t GetSectorIndex(const Float3& position, const MapFormat& map)
//     // {
//     //     const int32_t x = std::floor(static_cast<float>(position.x)) / SECTOR_DIM;
//     //     const int32_t z = std::floor(static_cast<float>(position.z)) / SECTOR_DIM;

//     //     return z * map.HorizontalSectors() + x;
//     // }

//     void NavigationSystem::RunSync(Registry * registry, CommandBuffer & cmd, uint32_t tick)
//     {
//         // auto& reg = registry->Get();
//         // auto view = reg.view<PathRequest>();

//         // const MapFormat& map = registry->GetResource<MapFormat>();
//         // const PortalMap& portalMap = registry->GetResource<PortalMap>();
//         // PathNetworkMap& pathNetwork = registry->GetResource<PathNetworkMap>();

//         // auto func = [&](entt::entity entity, const PathRequest& request)
//         // {
//         //     const Float3& origin = request.origin;
//         //     const Float3& destination = request.destination;

//         //     const int32_t originSector = GetSectorIndex(origin, map);
//         //     const int32_t destinationSector = GetSectorIndex(destination, map);

//         //     if (IsSectorIndexValid(originSector, map) && IsSectorIndexValid(destinationSector, map))
//         //     {
//         //         const std::vector<int32_t> path = AStar(originSector, destinationSector, map, portalMap);
//         //         if (path.size() > 0)
//         //         {
//         //             pathNetwork.networks[originSector] = PathNetwork{ path, 0.0f };
//         //         }
//         //     }

//         //     cmd.Destroy(entity);
//         // };

//         // view.each(func);
//     }