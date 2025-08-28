// Graph.cpp
#include "Graph.hpp"
#include "tinyxml2.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <queue>
#include <set>
#include <unordered_set>
#include <stack>
#include <limits>
#include <list>

using namespace tinyxml2;

// helper functions in anonymous namespace
namespace {
    constexpr double EARTH_RADIUS = 6378137.0;
    constexpr double DEG_TO_RAD  = 3.14159265358979323846 / 180.0;

    double toRadians(double degrees) {
        return degrees * DEG_TO_RAD;
    }

    double haversineDistance(double lat1, double lon1,
                         double lat2, double lon2) {
    double lat1Rad   = toRadians(lat1);
    double lat2Rad   = toRadians(lat2);
    double deltaLat  = toRadians(lat2 - lat1);
    double deltaLon  = toRadians(lon2 - lon1);

    double a = std::sin(deltaLat/2) * std::sin(deltaLat/2)
             + std::cos(lat1Rad) * std::cos(lat2Rad)
               * std::sin(deltaLon/2) * std::sin(deltaLon/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return EARTH_RADIUS * c;
}

    double highwayFactor(const std::string& h) {
        if (h == "motorway"   || h == "trunk")        return 0.5;
        if (h == "primary"    || h == "secondary")    return 0.75;
        if (h == "tertiary"   || h == "residential")  return 1.0;
        if (h == "living_street" || h == "unclassified") return 1.25;
        if (h == "service"    || h == "track")        return 1.5;
        return 1.0;
    }
}

// Default constructor
Graph::Graph()
    : vertices(), idToIndex()
{}

Graph::Graph(const std::string& osmFilename)
    : vertices(), idToIndex()
{
    XMLDocument doc;
    if (doc.LoadFile(osmFilename.c_str()) != XML_SUCCESS) {
        return; // failed to load file
    }
    XMLElement* root = doc.FirstChildElement("osm");
    if (!root) return;

    // 1) Read all <node> entries
    for (XMLElement* elem = root->FirstChildElement("node");
         elem;
         elem = elem->NextSiblingElement("node"))
    {
        unsigned long id  = (unsigned long)elem->Unsigned64Attribute("id");
        double lat        = elem->DoubleAttribute("lat");
        double lon        = elem->DoubleAttribute("lon");
        addVertex(id, lat, lon);
    }

    // 2) Read all <way> entries with a highway tag
    for (XMLElement* way = root->FirstChildElement("way");
         way;
         way = way->NextSiblingElement("way"))
    {
        // find highway tag
        const char* highway = nullptr;
        for (XMLElement* tag = way->FirstChildElement("tag");
             tag;
             tag = tag->NextSiblingElement("tag"))
        {
            if (std::string(tag->Attribute("k")) == "highway") {
                highway = tag->Attribute("v");
                break;
            }
        }
        if (!highway) continue;

        // only real road types — skip pedestrian, bus_stop, etc.
        static const std::unordered_set<std::string> allowed = {
            "motorway","trunk",
            "primary","secondary",
            "tertiary","residential",
            "living_street","unclassified",
            "service","track", "pedestrian"
        };
        if (allowed.count(highway) == 0) 
            continue;

        const char* onewayAttr = way->Attribute("oneway");
        bool oneway = onewayAttr
                    && (std::string(onewayAttr) == "yes"
                     || std::string(onewayAttr) == "1");
        double factor = highwayFactor(highway);

        // collect node references
        std::vector<unsigned long> refs;
        for (XMLElement* nd = way->FirstChildElement("nd");
             nd;
             nd = nd->NextSiblingElement("nd"))
        {
            unsigned long refId = (unsigned long)nd->Unsigned64Attribute("ref");
            if (idToIndex.count(refId)) {
                refs.push_back(refId);
            }
        }

        // build edges between consecutive refs
        for (size_t i = 0; i + 1 < refs.size(); ++i) {
            unsigned long u = refs[i], v = refs[i+1];
            Vertex& Vu = vertices[idToIndex[u]];
            Vertex& Vv = vertices[idToIndex[v]];

            double dist = haversineDistance(
                Vu.getLatitude(), Vu.getLongitude(),
                Vv.getLatitude(), Vv.getLongitude()
            ) * factor;

            Vu.addEdge(Edge(u, v, dist));
            if (!oneway) {
                Vv.addEdge(Edge(v, u, dist));
            }
        }
    }

    // 3) Remove all disconnected vertices
    std::vector<unsigned long> toRemove;
    for (const Vertex& vert : vertices) {
        if (vert.getEdges().empty()) {
            unsigned long vid = vert.getId();
            bool hasIncoming = false;
            for (const Vertex& other : vertices) {
                for (const Edge& e : other.getEdges()) {
                    if (e.getDestinationId() == vid) {
                        hasIncoming = true;
                        break;
                    }
                }
                if (hasIncoming) break;
            }
            if (!hasIncoming) {
                toRemove.push_back(vid);
            }
        }
    }
    for (unsigned long vid : toRemove) {
        removeVertex(vid);
    }
}

// Add a vertex; returns false if id already exists
bool Graph::addVertex(unsigned long id, double latitude, double longitude) {
    if (idToIndex.find(id) != idToIndex.end()) {
        return false;
    }
    vertices.emplace_back(id, latitude, longitude);
    idToIndex[id] = static_cast<unsigned int>(vertices.size() - 1);
    return true;
}

// Remove a vertex and all edges to/from it; returns false if id not found
bool Graph::removeVertex(unsigned long id) {
    auto it = idToIndex.find(id);
    if (it == idToIndex.end()) {
        return false;
    }
    unsigned int index = it->second;

    // Remove all outgoing edges from this vertex
    vertices[index].getEdges();  // just to ensure edges exist
    while (!vertices[index].getEdges().empty()) {
        unsigned long dest = vertices[index].getEdges().front().getDestinationId();
        vertices[index].removeEdge(dest);
    }

    // Remove all incoming edges to this vertex
    for (unsigned int i = 0; i < vertices.size(); ++i) {
        if (i == index) continue;
        vertices[i].removeEdge(id);
    }

    // Erase the vertex
    vertices.erase(vertices.begin() + index);
    idToIndex.erase(it);

    // Update indices of vertices that shifted left
    for (auto& pair : idToIndex) {
        if (pair.second > index) {
            --pair.second;
        }
    }

    return true;
}

// Dijkstra's algorithm for shortest path
std::list<unsigned long> Graph::findShortestPath(unsigned long startId,
                                                 unsigned long endId) const {
    // Check both vertices exist
    auto sit = idToIndex.find(startId);
    auto eit = idToIndex.find(endId);
    if (sit == idToIndex.end() || eit == idToIndex.end()) {
        return {};
    }

    // Min-heap node
    struct Node {
        double dist;
        unsigned long id;
    };
    struct Compare {
        bool operator()(Node const& a, Node const& b) const {
            if (a.dist != b.dist)
                return a.dist > b.dist;
            return a.id > b.id;
        }
    };

    // Initialize distances
    std::unordered_map<unsigned long, double> dist;
    dist.reserve(vertices.size());
    for (auto const& pair : idToIndex) {
        dist[pair.first] = std::numeric_limits<double>::infinity();
    }
    dist[startId] = 0.0;

    // Previous vertex map
    std::unordered_map<unsigned long, unsigned long> prev;

    // Priority queue
    std::priority_queue<Node, std::vector<Node>, Compare> pq;
    pq.push({0.0, startId});

    while (!pq.empty()) {
        Node node = pq.top();
        pq.pop();

        // Skip stale entry
        if (node.dist > dist[node.id]) continue;

        // Stop if reached destination
        if (node.id == endId) break;

        // Explore neighbors
        auto const& edges = vertices.at(idToIndex.at(node.id)).getEdges();
        for (auto const& e : edges) {
            unsigned long v = e.getDestinationId();
            double alt = node.dist + e.getDistance();
            if (alt < dist[v]) {
                dist[v] = alt;
                prev[v] = node.id;
                pq.push({alt, v});
            }
        }
    }

    // Reconstruct path
    std::list<unsigned long> path;
    if (dist[endId] == std::numeric_limits<double>::infinity()) {
        return {};  // no path
    }
    for (unsigned long at = endId; ; at = prev[at]) {
        path.push_front(at);
        if (at == startId) break;
    }
    return path;
}

void Graph::compressGraph() {
    struct NewEdge { unsigned long src, dst; double dist; };

    // build incoming/outgoing
    std::unordered_map<unsigned long, std::vector<std::pair<unsigned long,double>>> incoming, outgoing;
    for (auto const& v : vertices) {
        incoming[v.getId()] = {};
        outgoing[v.getId()] = {};
    }
    for (auto const& v : vertices) {
        for (auto const& e : v.getEdges()) {
            outgoing[v.getId()].emplace_back(e.getDestinationId(), e.getDistance());
            incoming[e.getDestinationId()].emplace_back(v.getId(), e.getDistance());
        }
    }

    std::vector<NewEdge> edgesToAdd;
    std::vector<unsigned long> nodesToRemove;

    for (auto const& v : vertices) {
        auto vid = v.getId();
        auto& ins = incoming[vid];
        auto& outs = outgoing[vid];

        // one-way compression
        if (ins.size()==1 && outs.size()==1 && ins[0].first!=outs[0].first) {
            if (ins.size()+outs.size()==2) {
                auto from = ins[0].first;
                auto to   = outs[0].first;
                double nd = ins[0].second + outs[0].second;
                edgesToAdd.push_back(NewEdge{from,to,nd});
                nodesToRemove.push_back(vid);
            }
        }
        // two-way compression
        else if (ins.size()==2 && outs.size()==2) {
            std::set<unsigned long> inSet, outSet;
            for (auto const& p: ins)  inSet.insert(p.first);
            for (auto const& p: outs) outSet.insert(p.first);
            if (inSet==outSet && ins.size()+outs.size()==4) {
                for (auto a: inSet) {
                    double distIn=0, distOut=0;
                    for (auto const& p: ins)  if(p.first==a) distIn=p.second;
                    for (auto const& p: outs) if(p.first==a) continue;
                    for (auto b: inSet) {
                        if (b==a) continue;
                        for (auto const& p: outs) if(p.first==b) distOut=p.second;
                        edgesToAdd.push_back(NewEdge{a,b,distIn+distOut});
                    }
                }
                nodesToRemove.push_back(vid);
            }
        }
    }

    // add new edges
    for (auto const& ne: edgesToAdd) {
        auto it = idToIndex.find(ne.src);
        if (it!=idToIndex.end()) {
            vertices[it->second].addEdge(Edge(ne.src, ne.dst, ne.dist));
        }
    }
    // remove nodes
    for (auto nid: nodesToRemove) {
        removeVertex(nid);
    }
}

std::list<unsigned long> Graph::breadthFirstSearch(unsigned long startId) const {
    std::list<unsigned long> order;
    auto startIt = idToIndex.find(startId);
    if (startIt == idToIndex.end()) {
        // even if it was pruned, print it once
        return std::list<unsigned long>{ startId };
    }

    std::unordered_set<unsigned long> visited;
    std::queue<unsigned long> q;

    visited.insert(startId);
    q.push(startId);

    while (!q.empty()) {
        unsigned long current = q.front();
        q.pop();
        order.push_back(current);

        // Collect neighbors
        const auto& edges = vertices.at(idToIndex.at(current)).getEdges();
        std::vector<unsigned long> neighbors;
        neighbors.reserve(edges.size());
        for (const auto& e : edges) {
            neighbors.push_back(e.getDestinationId());
        }
        std::sort(neighbors.begin(), neighbors.end());

        // Enqueue unvisited neighbors
        for (auto nid : neighbors) {
            if (visited.insert(nid).second) {
                q.push(nid);
            }
        }
    }

    return order;
}

std::list<unsigned long> Graph::depthFirstSearch(unsigned long startId) const {
    std::list<unsigned long> order;
    auto startIt = idToIndex.find(startId);
    if (startIt == idToIndex.end()) {
        // even if it was pruned, print it once
        return std::list<unsigned long>{ startId };
    }

    std::unordered_set<unsigned long> visited;
    std::stack<unsigned long> stk;
    stk.push(startId);

    while (!stk.empty()) {
        unsigned long current = stk.top();
        stk.pop();
        if (!visited.insert(current).second) {
            continue;  // already visited
        }
        order.push_back(current);

        // Collect neighbors
        const auto& edges = vertices.at(idToIndex.at(current)).getEdges();
        std::vector<unsigned long> neighbors;
        neighbors.reserve(edges.size());
        for (const auto& e : edges) {
            neighbors.push_back(e.getDestinationId());
        }
        std::sort(neighbors.begin(), neighbors.end());

        // Push in reverse order so smallest ID is processed first
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
            unsigned long nid = *it;
            if (visited.find(nid) == visited.end()) {
                stk.push(nid);
            }
        }
    }

    return order;
}

// Return all vertices
const std::vector<Vertex>& Graph::getVertices() const {
    return vertices;
}

// Return the id-to-index map
const std::unordered_map<unsigned long, unsigned int>& Graph::getIdToIndex() const {
    return idToIndex;
}