// Vertex.cpp
#include "Vertex.hpp"
#include <algorithm>

// Constructor
Vertex::Vertex(unsigned long id, double latitude, double longitude)
    : id(id), latitude(latitude), longitude(longitude)
{}

// Copy constructor
Vertex::Vertex(const Vertex& other)
    : id(other.id),
      latitude(other.latitude),
      longitude(other.longitude),
      edges(other.edges)
{}

// Add an outgoing edge
void Vertex::addEdge(const Edge& edge) {
    edges.push_back(edge);
}

// Remove an outgoing edge by destination id; returns true if removed
bool Vertex::removeEdge(unsigned long destinationId) {
    auto it = std::find_if(
        edges.begin(), edges.end(),
        [destinationId](const Edge& e) {
            return e.getDestinationId() == destinationId;
        }
    );
    if (it != edges.end()) {
        edges.erase(it);
        return true;
    }
    return false;
}

// Getters
unsigned long Vertex::getId() const {
    return id;
}

double Vertex::getLatitude() const {
    return latitude;
}

double Vertex::getLongitude() const {
    return longitude;
}

const std::list<Edge>& Vertex::getEdges() const {
    return edges;
}