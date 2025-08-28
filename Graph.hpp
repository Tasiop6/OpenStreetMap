// Graph.h
#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <unordered_map>
#include <string>
#include <list>
#include "Vertex.hpp"

class Graph {
private:
    std::vector<Vertex> vertices;                             // all graph vertices
    std::unordered_map<unsigned long, unsigned int> idToIndex; // map vertex id -> index in vertices

public:
    // Default constructor
    Graph();

    // Construct graph from an OSM XML file, then remove all disconnected vertices
    Graph(const std::string& osmFilename);

    // Add a vertex; returns false if id already exists
    bool addVertex(unsigned long id, double latitude, double longitude);

    // Remove a vertex and all edges to/from it; returns false if id not found
    bool removeVertex(unsigned long id);

    // Find shortest path (Dijkstra). Returns list of vertex IDs from start to end.
    std::list<unsigned long> findShortestPath(unsigned long startId,
                                              unsigned long endId) const;

    // Compress the graph by removing intermediate nodes on one- and two-way roads
    void compressGraph();

     // BFS traversal: returns vertex IDs in the order visited
    std::list<unsigned long> breadthFirstSearch(unsigned long startId) const;

    // DFS traversal: returns vertex IDs in the order visited
    std::list<unsigned long> depthFirstSearch(unsigned long startId) const;

    // Getters
    const std::vector<Vertex>& getVertices() const;
    const std::unordered_map<unsigned long, unsigned int>& getIdToIndex() const;
};

#endif // GRAPH_H