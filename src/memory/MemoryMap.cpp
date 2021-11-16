#include "memory/MemoryMap.hpp"

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

namespace toygb {
	// MemoryMap

	MemoryMap::MemoryMap(){
		m_tree = nullptr;
	}

	MemoryMap::~MemoryMap(){
		if (m_tree != nullptr)
			delete[] m_tree;
	}

	void MemoryMap::add(uint16_t start, uint16_t end, MemoryMapping* mapping){
		MemoryMap::Node node(start, end, mapping);
		m_array.push_back(node);
		//std::cout << oh16(node.start) << " - " << oh16(node.end) << " : " << typeid(*(node.mapping)).name() << std::endl;
	}

	void MemoryMap::build(){
		if (m_tree != nullptr) delete[] m_tree;
		m_tree = new MemoryMap::Node[m_array.size()];

		std::sort(m_array.begin(), m_array.end(), MemoryMap::NodeComparator());

		m_treesize = m_array.size();
		buildSubtree(0, 0, m_array.size());

		/*std::cout << std::endl << std::endl;
		for (unsigned int i = 0; i < m_array.size(); i++){
			Node node = m_array[i];
			std::cout << oh16(node.start) << " - " << oh16(node.end) << " : " << typeid(*(node.mapping)).name() << " " << node.mapping << std::endl;
		}
		std::cout << std::endl << std::endl;
		for (unsigned int i = 0; i < m_array.size(); i++){
			Node node = m_tree[i];
			std::cout << oh16(node.start) << " - " << oh16(node.end) << " : " << typeid(*(node.mapping)).name() << " " << node.mapping << std::endl;
		}*/
	}

	void MemoryMap::buildSubtree(int rootindex, int startindex, int endindex){
		int size = endindex - startindex;

		if (size == 0) return;
		if (size == 1){
			m_tree[rootindex] = m_array[startindex];
			return;
		}

		int depth = (int)std::floor(std::log2(size));
		int base = (1 << depth) - 1;
		int sidebase = (1 << (depth - 1)) - 1;
		int lastrow = size - base;

		int leftsize = sidebase + min(lastrow, sidebase + 1);
		//int rightsize = sidebase + max(0, lastrow - (sidebase + 1));

		m_tree[rootindex] = m_array[startindex + leftsize];
		buildSubtree(2*rootindex + 1, startindex, startindex + leftsize);
		buildSubtree(2*rootindex + 2, startindex + leftsize + 1, endindex);
	}

	MemoryMap::Node* MemoryMap::getNode(uint16_t address){
		int index = 0;
		MemoryMap::Node* found = nullptr;
		while (found == nullptr && index < m_treesize){
			MemoryMap::Node* node = &(m_tree[index]);
			if (node->start <= address){
				if (address <= node->end){
					found = node;
				} else {
					index = 2*index + 2;  // right
				}
			} else {
				index = 2*index + 1;  // left
			}
		}

		return found;
	}


	uint8_t MemoryMap::get(uint16_t address){
		//std::cout << "Read from " << oh16(address);
		MemoryMap::Node* node = getNode(address);
		if (node != nullptr){
			//std::cout << " OK" << std::endl;
			return node->mapping->get(address - node->start);
		} else {
			//std::cout << " Fail" << std::endl;
			//std::stringstream errstream;
			//errstream << "Read from unmapped memory address " << oh16(address);
			//throw EmulationError(errstream.str());
			std::cout << "Read from unmapped memory address " << oh16(address) << std::endl;
			return 0xFF;
		}
	}

	void MemoryMap::set(uint16_t address, uint8_t value){
		//std::cout << "Write " << oh8(value) << " to " << oh16(address);
		MemoryMap::Node* node = getNode(address);
		if (node != nullptr){
			//std::cout << " OK" << std::endl;
			return node->mapping->set(address - node->start, value);
		} else {
			//std::cout << " Fail" << std::endl;
			std::stringstream errstream;
			errstream << "Write to unmapped memory address : " << oh16(address);
			throw EmulationError(errstream.str());
		}
	}

	// MemoryMap::Node

	MemoryMap::Node::Node(){
		mapping = nullptr;
	}

	MemoryMap::Node::Node(uint16_t mapstart, uint16_t mapend, MemoryMapping* map){
		start = mapstart;
		end = mapend;
		mapping = map;
	}

	MemoryMap::Node::Node(MemoryMap::Node const& copied){
		start = copied.start;
		end = copied.end;
		mapping = copied.mapping;
	}

	MemoryMap::Node& MemoryMap::Node::operator=(MemoryMap::Node const& copied){
		start = copied.start;
		end = copied.end;
		mapping = copied.mapping;
		return *this;
	}

	// MemoryMap::NodeComparator

	bool MemoryMap::NodeComparator::operator()(MemoryMap::Node& node1, MemoryMap::Node& node2){
		return node1.start < node2.start;
	}
}
