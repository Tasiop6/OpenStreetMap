// HW5.cpp
#include <iostream>
#include <iomanip>
#include <string>
#include <iterator>
#include "Graph.hpp"
#include "tinyxml2.h"
#include <limits>
#include <sstream>

int main() {
    Graph graph;
    std::string line;

    while (true) {
        // One blank line, then prompt (with its own newline)
        std::cout << "\nEnter your choice: \n";

        // Read the entire command line
        if (!std::getline(std::cin, line))
            return 0;

        // Skip empty or malformed lines
        if (line.size() < 2 || line[0] != '-')
            continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "-q") {
            return 0;
        }
        else if (cmd == "-i") {
            std::string filepath;
            iss >> filepath;

            tinyxml2::XMLDocument doc;
            auto err = doc.LoadFile(filepath.c_str());
            if (err != tinyxml2::XML_SUCCESS) {
                if (err == tinyxml2::XML_ERROR_FILE_NOT_FOUND
                 || err == tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED)
                {
                    std::cout << "Unable to open file: " << filepath << '\n';
                } else {
                    std::cout << "Invalid format for file: " << filepath << '\n';
                }
            } else {
                graph = Graph(filepath);
                std::cout << "Graph OK\n";
            }
        }
        else if (cmd == "-c") {
            graph.compressGraph();
            // Blank before success
            std::cout << '\n';
            std::cout << "Compact OK\n";
        }
        else if (cmd == "-p") {
            unsigned long sid, eid;
            iss >> sid >> eid;
            auto pathList = graph.findShortestPath(sid, eid);
            double cumulative = 0.0;

            // Print edges with cumulative distances
            for (auto it = pathList.begin(), nxt = std::next(it);
                 nxt != pathList.end();
                 ++it, ++nxt)
            {
                unsigned long u = *it, v = *nxt;
                double d = 0.0;
                for (auto const& e : graph
                       .getVertices()[ graph.getIdToIndex().at(u) ]
                       .getEdges())
                {
                    if (e.getDestinationId() == v) {
                        d = e.getDistance();
                        break;
                    }
                }
                cumulative += d;
                std::cout
                  << '[' << u << " -> " << v << "] "
                  << std::fixed << std::setprecision(3)
                  << cumulative << '\n';
            }

            // Two blanks, then Google Maps URL
            std::cout << "\n\n";
            std::cout << "https://www.google.com/maps/dir/";
            for (auto it = pathList.begin(); it != pathList.end(); ++it) {
                auto const& vert = graph
                  .getVertices()[ graph.getIdToIndex().at(*it) ];
                std::cout
                  << vert.getLatitude() << ',' << vert.getLongitude()
                  << '/';
            }
            std::cout << '\n';
        }
        else if (cmd == "-b") {
            unsigned long sid;
            iss >> sid;
            auto order = graph.breadthFirstSearch(sid);
            for (auto id : order) {
                std::cout << id << '\n';
            }
        }
        else if (cmd == "-d") {
            unsigned long sid;
            iss >> sid;
            auto order = graph.depthFirstSearch(sid);
            for (auto id : order) {
                std::cout << id << '\n';
            }
        }
        // any other input is ignored
    }

    return 0;
}