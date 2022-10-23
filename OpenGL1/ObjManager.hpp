#pragma once


#ifndef OBJ_MANAGER_H
#define OBJ_MANAGER_H


#include<vector>
template<class T>
struct ObjManager {
	std::vector<T> objs;

	void addObj(T& t) {
		objs.push_back(t);
	}
};
#endif // !OBJ_MANAGER_H
