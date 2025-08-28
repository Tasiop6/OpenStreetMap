// Edge.cpp
#include "Edge.hpp"

// Constructor
Edge::Edge(unsigned long sourceId,
           unsigned long destinationId,
           double distance)
    : sourceId(sourceId),
      destinationId(destinationId),
      distance(distance)
{}

// Copy constructor
Edge::Edge(const Edge& other)
    : sourceId(other.sourceId),
      destinationId(other.destinationId),
      distance(other.distance)
{}

// Getters
unsigned long Edge::getSourceId() const {
    return sourceId;
}

unsigned long Edge::getDestinationId() const {
    return destinationId;
}

double Edge::getDistance() const {
    return distance;
}

// Setters
void Edge::setSourceId(unsigned long sourceId) {
    this->sourceId = sourceId;
}

void Edge::setDestinationId(unsigned long destinationId) {
    this->destinationId = destinationId;
}

void Edge::setDistance(double distance) {
    this->distance = distance;
}