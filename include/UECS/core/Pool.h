#pragma once
#pragma once

#include <vector>
#include <array>
#include <unordered_set>

// no release
namespace Ubpa {
	template<typename T>
	class Pool {
	public:
		~Pool() { clear(); }

	public:
		T* const request() {
			if (freeAdresses.empty())
				NewBlock();
			T* freeAdress = freeAdresses.back();
			freeAdresses.pop_back();
			return freeAdress;
		}

		void recycle(T* object) {
			freeAdresses.push_back(object);
		}

		void reserve(size_t n) {
			size_t blockNum = n / BLOCK_SIZE + (n % BLOCK_SIZE > 0 ? 1 : 0);
			for (size_t i = blocks.size(); i < blockNum; i++)
				NewBlock();
		}

		void clear() {
			std::unordered_set<T*> freeAdressesSet(freeAdresses.begin(), freeAdresses.end());
			for (auto block : blocks) {
				/*for (size_t i = 0; i < BLOCK_SIZE; i++) {
					T* adress = block->data() + i;
					if (freeAdressesSet.find(adress) == freeAdressesSet.end())
						adress->~T();
				}*/
				free(block);
			}
			blocks.clear();
			freeAdresses.clear();
		}

	private:
		void NewBlock() {
			auto block = (Block*)malloc(sizeof(Block)); // won't call constructor
			blocks.push_back(block);
			for (size_t i = 0; i < BLOCK_SIZE; i++)
				freeAdresses.push_back(block->data() + i);
		}

	private:
		static const size_t BLOCK_SIZE = 1024;
		using Block = std::array<T, BLOCK_SIZE>;

		std::vector<Block*> blocks;
		std::vector<T*> freeAdresses;
	};
}
