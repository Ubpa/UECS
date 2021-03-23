#pragma once

#include <memory_resource>
#include <iostream>
#include <unordered_set>

class MyResource : public std::pmr::unsynchronized_pool_resource {
public:
	bool verbose{ true };

	using std::pmr::unsynchronized_pool_resource::unsynchronized_pool_resource;

	void release() noexcept /* strengthened */ {
		if (!buffers.empty()) {
			std::cout << "!!! memory leak !!!" << std::endl;
			for (void* buffer : buffers)
				std::cout << "  leak @" << buffer << std::endl;
		}
		else
			std::cout << "Great! No memory leak!" << std::endl;
		this->unsynchronized_pool_resource::release();
	}

protected:
	virtual void* do_allocate(const size_t _Bytes, const size_t _Align) override {
		void* _Ptr = this->unsynchronized_pool_resource::do_allocate(_Bytes, _Align);
		if(verbose)
			std::cout << "  allocate @" << _Ptr << " (" << _Bytes << ", " << _Align << ")" << std::endl;
		buffers.insert(_Ptr);
		return _Ptr;
	}

	virtual void do_deallocate(void* const _Ptr, const size_t _Bytes, const size_t _Align) override {
		if (verbose)
			std::cout << "deallocate @" << _Ptr << " (" << _Bytes << ", " << _Align << ")" << std::endl;
		buffers.erase(_Ptr);
		this->unsynchronized_pool_resource::do_deallocate(_Ptr, _Bytes, _Align);
	}
private:
	std::unordered_set<void*> buffers;
};
