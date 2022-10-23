
#ifndef GC__H
#define GC__H
#include<vector>
#include"gc_type.hpp"
#include<iostream>
#include<list>
#include<map>

template<class T>
class AutoGC {
private:
	T* t;
	int* refCount;
public:
	AutoGC(T* t);
	AutoGC(AutoGC& gc);
	AutoGC(AutoGC&& gc) noexcept;
	AutoGC& operator=(AutoGC&);
	T* operator->();
	~AutoGC();
	friend class GCController;
};

struct GCInfo
{
	GCType type;
	gc_level level;
};

class GCController {
public:
	GCController() :id('c'){}

	~GCController() {
		for (auto i = all.begin(); i != all.end(); ++i) {
			delete i->first;
		}
	}

	void update() {
		std::vector<decltype(all.begin())> v;
		for (auto i = all.begin(); i != all.end(); ++i) {
			if (i->second.type==PendingKill) {
				v.push_back(i);
			}
		}

		for (auto& i : v) {
			delete i->first;
			all.erase(i);
		}
	}

	void SetGCType(void* v, GCType t) {
		for (auto i = all.begin(); i != all.end(); ++i) {
			if (i->first == v) {
				i->second.type = t;
				return;
			}
		}
	}

	std::map<void*,GCInfo> all;
	uint8_t id;
	GCController(GCController&) = delete;
	GCController(GCController&&) = delete;
};

extern GCController gcController;
void test();
#endif // !GC_CONTROLLER_H
