// Vertex.h
#ifndef VERTEX_H
#define VERTEX_H

#include <list>
#include "Edge.hpp"

class Vertex {
private:
    unsigned long id;            // unique vertex identifier
    double latitude;             // geographic latitude
    double longitude;            // geographic longitude
    std::list<Edge> edges;       // outgoing edges

public:
    // Constructor
    Vertex(unsigned long id, double latitude, double longitude);

    // Copy constructor
    Vertex(const Vertex& other);

    // Add an outgoing edge
    void addEdge(const Edge& edge);

    // Remove an outgoing edge by destination id; returns true if removed
    bool removeEdge(unsigned long destinationId);

    // Getters
    unsigned long getId() const;
    double getLatitude() const;
    double getLongitude() const;
    const std::list<Edge>& getEdges() const;
};

#endif // VERTEX_H