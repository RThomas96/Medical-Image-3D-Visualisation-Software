#ifndef OctreePoint_H
#define OctreePoint_H

#include <glm/glm.hpp>

// Simple point data type to insert into the tree.
// Have something with more interesting behavior inherit
// from this in order to store other attributes in the tree.
class OctreePoint {
    int idx;
	glm::vec3 position; 
public:
	OctreePoint() { }
	OctreePoint(const glm::vec3& position) : position(position) { }
	inline const glm::vec3& getPosition() const { return position; }
	inline void setPosition(const glm::vec3& p) { position = p; }
	inline const int& getIdx() const { return idx; }
	inline void setIdx(const int& p) { idx = p; }
};

#endif
