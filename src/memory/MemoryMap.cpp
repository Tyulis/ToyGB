#include "memory/MemoryMap.hpp"

// Comparison utilities
#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

namespace toygb {

	// Initialize the memory map
	MemoryMap::MemoryMap(){
		m_tree = nullptr;
	}

	MemoryMap::~MemoryMap(){
		if (m_tree != nullptr) delete[] m_tree;
		m_tree = nullptr;
	}

	// Configure a memory mapping with its start and end absolute addresses (INCLUDED), and put it in the temporary array
	void MemoryMap::add(uint16_t start, uint16_t end, MemoryMapping* mapping) {
		MemoryMap::Node node(start, end, mapping);
		m_array.push_back(node);
	}

	// Build the final binary search tree with all mappings configured in the array
	void MemoryMap::build() {
		if (m_tree != nullptr) delete[] m_tree;

		m_treesize = m_array.size();
		m_tree = new MemoryMap::Node[m_treesize];

		std::sort(m_array.begin(), m_array.end(), MemoryMap::NodeComparator());
		buildSubtree(0, 0, m_array.size());
	}

	// Recursively build the binary search tree from the sorted temporary array
	void MemoryMap::buildSubtree(int rootindex, int startindex, int endindex) {
		int size = endindex - startindex;

		if (size == 0) return;
		if (size == 1){
			m_tree[rootindex] = m_array[startindex];
			return;
		}

		int depth = int(std::floor(std::log2(size)));
		int base = (1 << depth) - 1;
		int sidebase = (1 << (depth - 1)) - 1;
		int lastrow = size - base;

		int leftsize = sidebase + min(lastrow, sidebase + 1);
		//int rightsize = sidebase + max(0, lastrow - (sidebase + 1));

		m_tree[rootindex] = m_array[startindex + leftsize];
		buildSubtree(2*rootindex + 1, startindex, startindex + leftsize);
		buildSubtree(2*rootindex + 2, startindex + leftsize + 1, endindex);
	}

	// Get the node to dispatch the given address to, or nullptr if none found
	MemoryMap::Node* MemoryMap::getNode(uint16_t address) {
		int index = 0;
		MemoryMap::Node* found = nullptr;
		while (found == nullptr && index < m_treesize){
			MemoryMap::Node* node = &(m_tree[index]);
			if (node->start <= address) {  // Node start <= searched address -> need to control the end address
				if (address <= node->end)  // Node start <= searched address <= node end -> found
					found = node;
				else  // Node end < searched address -> search on the right (>) side
					index = 2*index + 2;  // right
			} else {  // Searched address < Node start -> search on the left (<) side
				index = 2*index + 1;  // left
			}
		}

		return found;
	}


	// Dispatch a memory read to the corresponding memory mapping
	uint8_t MemoryMap::get(uint16_t address) {
		MemoryMap::Node* node = getNode(address);
		if (node != nullptr){
			return node->mapping->get(address - node->start);
		} else {
			// No errors, the gameboy can read from unmapped memory addresses, it just returns nothing (0xFF)
			// FIXME : Currently issue a warning just in case, should be a debugging parameter or something
			std::cerr << "Read from unmapped memory address " << oh16(address) << std::endl;
			return 0xFF;
		}
	}

	// Dispatch a memory write to the corresponding memory mapping
	void MemoryMap::set(uint16_t address, uint8_t value) {
		// In the real hardware, FF60 is a test IO register (only accessible when CPU test pins are not grounded)
		// So for quick debugging, let's log anything that gets written there
		if (address == 0xFF60) {
			std::cout << "DEBUG : Write to $FF60 : " << oh8(value) << std::endl;
			return;
		}

		MemoryMap::Node* node = getNode(address);
		if (node != nullptr){
			return node->mapping->set(address - node->start, value);
		} else {
			// No errors, the gameboy can try to write to unmapped memory addresses, it just gets ignored
			// FIXME : Currently issue a warning just in case, should be a debugging parameter or something
			std::cout << "Write to unmapped memory address : " << oh16(address) << std::endl;
		}
	}

	// Get the memory mapping that handles the given address
	MemoryMapping* MemoryMap::getMapping(uint16_t address) {
		MemoryMap::Node* node = getNode(address);
		return node->mapping;
	}

	////////// MemoryMap::Node

	MemoryMap::Node::Node() {
		mapping = nullptr;
	}

	MemoryMap::Node::Node(uint16_t mapstart, uint16_t mapend, MemoryMapping* map) {
		start = mapstart;
		end = mapend;
		mapping = map;
	}

	MemoryMap::Node::Node(MemoryMap::Node const& copied) {
		start = copied.start;
		end = copied.end;
		mapping = copied.mapping;
	}

	MemoryMap::Node& MemoryMap::Node::operator=(MemoryMap::Node const& copied) {
		start = copied.start;
		end = copied.end;
		mapping = copied.mapping;
		return *this;
	}

	////////// MemoryMap::NodeComparator

	bool MemoryMap::NodeComparator::operator()(MemoryMap::Node& node1, MemoryMap::Node& node2) {
		return node1.start < node2.start;
	}
}
