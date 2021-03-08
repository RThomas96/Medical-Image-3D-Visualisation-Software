#ifndef LINKED_IMAGE_CACHE_HPP
#define LINKED_IMAGE_CACHE_HPP

#include <bits/c++config.h>
#include <vector>

#define MAX_CACHED_SLICES 10

struct node {
	std::size_t index;
	void* data;
};

// creates a new node on the heap
node* new_node(std::size_t i, void* d) {
	node* n = (node*)malloc(sizeof(node));
	n->index = i; n->data = d;
	return n;
}
// frees node contents
void delete_node_contents(node* n) {
	free(n->data);
}

struct slice_cache {
	std::size_t last;
	std::vector<node*> slices;
	slice_cache() {
		last = 0;
		slices.clear();
	}
	// does the cache have the slice 'n' ?
	bool has_slice(std::size_t i) {
		return (slices.size() == 0) ? false : std::find(std::begin(slices), std::end(slices),
			[i](node* t) -> bool {
				return t->index == i;
			}) != std::end(slices);
	}
	// add a slice to the cache :
	node* add_slice(void* data, std::size_t sliceidx) {
		// nn = new_node
		node* nn = new_node(sliceidx, data);
		// rn = return_node
		node* rn = nullptr;
		// if we havent reached limit, just add data
		if (slices.size() < MAX_CACHED_SLICES) { slices.push_back(nn); last = slices.size()+1; }
		else {
			// remove slices[last] :
			rn = slices[last];
			// replace it with new node :
			slices[last] = nn;
			// increment last and wrap it if necessary :
			last = (last+1 < MAX_CACHED_SLICES) ? last+1 : 0 ;
		}
		return rn;
	}
	// destructor : cleans up the cache and deletes self
	~slice_cache() {
		for (node* slice : slices) {
			free(slice->data);
		}
		slices.clear();
	}
};

#endif // LINKED_IMAGE_CACHE_HPP
