#ifndef _MEMORY_MEMORYMAP_HPP
#define _MEMORY_MEMORYMAP_HPP

#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"

namespace toygb {
	class MemoryMap {
		public:
			MemoryMap();
			~MemoryMap();

			/**
			 * Configure a memory mapping
			 * @param start Start offset (included) from which to dispatch to the given mapping
			 * @param end End offset (included) to which to dispatch to the given mapping
			 * @param mapping Mapping to configure
			 */
			void add(uint16_t start, uint16_t end, MemoryMapping* mapping);
			void build();

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			MemoryMapping* getMapping(uint16_t address);

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

			class NodeComparator {
				public:
					bool operator()(MemoryMap::Node& node1, MemoryMap::Node& node2);
			};

		private:
			Node* getNode(uint16_t address);
			void buildSubtree(int rootindex, int startindex, int endindex);

			int m_treesize;
			Node* m_tree;
			std::vector<Node> m_array;
	};
}

#endif
