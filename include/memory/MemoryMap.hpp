#ifndef _MEMORY_MEMORYMAP_HPP
#define _MEMORY_MEMORYMAP_HPP

#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** General memory map, dispatches memory accesses to their respective memory mappings
	 *  Mappings are first configured and pushed into an array, then reorganised into a binary search tree for faster dispatch */
	class MemoryMap {
		public:
			MemoryMap();
			~MemoryMap();

			/** Configure a memory mapping with its start and end absolute addresses (both INCLUDED) */
			void add(uint16_t start, uint16_t end, MemoryMapping* mapping);

			/** Once all memory mappings have been configured, build the binary search tree
			 *  Must be called before attempting memory accesses, adding new mappings will not take effect before calling build() again */
			void build();

			// Memory accesses, take an absolute address and dispatch them to their respective memory mappings, giving them a relative address
			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			/** Get the memory mapping that handles accesses to the given absolute address */
			MemoryMapping* getMapping(uint16_t address);

			/** Tree node, contains all information necessary to handle a memory mapping (start and end addresses, and the actual mapping) */
			class Node {
				public:
					Node();
					Node(uint16_t start, uint16_t end, MemoryMapping* mapping);
					Node(Node const& copied);
					Node& operator=(Node const& copied);

					uint16_t start;
					uint16_t end;
					MemoryMapping* mapping;
			};

			/** Comparator, sorts mappings in order of increasing start address */
			class NodeComparator {
				public:
					bool operator()(MemoryMap::Node& node1, MemoryMap::Node& node2);
			};

		private:
			Node* getNode(uint16_t address);  // Get the node to dispatch the given address to
			void buildSubtree(int rootindex, int startindex, int endindex);  // Recursive function to build the binary search tree

			int m_treesize;  // Size of the tree array
			Node* m_tree;    // Binary search tree ordered by increasing start address, stored in an array (leftChild(index) = 2*index + 1, rightChild(index) = 2*index + 2)
			std::vector<Node> m_array;  // Temporary array to store configured mappings before building the tree
	};
}

#endif
