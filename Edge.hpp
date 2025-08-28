// Edge.h
#ifndef EDGE_H
#define EDGE_H

class Edge {
private:
    unsigned long sourceId;       // id of the source vertex (optional)
    unsigned long destinationId;  // id of the destination vertex
    double distance;              // distance between the two vertices

public:
    // Constructor
    Edge(unsigned long sourceId,
         unsigned long destinationId,
         double distance);

    // Copy constructor
    Edge(const Edge& other);

    // Getters
    unsigned long getSourceId() const;
    unsigned long getDestinationId() const;
    double getDistance() const;

    // Setters
    void setSourceId(unsigned long sourceId);
    void setDestinationId(unsigned long destinationId);
    void setDistance(double distance);
};

#endif // EDGE_H