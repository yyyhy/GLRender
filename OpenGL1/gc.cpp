#include "gc.hpp"

template<class T>
AutoGC<T>::AutoGC(T* t) :t(t),refCount(new int(1))
{
	gcController.all.emplace(t, GCInfo{ Live,0 });
}

template<class T>
AutoGC<T>::AutoGC(AutoGC<T>& gc)
{
	t = gc.t;
	refCount = gc.refCount;
	++*refCount;
}

template<class T>
AutoGC<T>::AutoGC(AutoGC<T>&& gc) noexcept
{
	t = std::move(gc.t);
	refCount = std::move(gc.refCount);
	++*refCount;
}

template<class T>
AutoGC<T>& AutoGC<T>::operator=(AutoGC<T>& gc)
{
	t = gc.t;
	--*refCount;
	refCount = gc.refCount;
	++*refCount;
	return *this;
}

template<class T>
T* AutoGC<T>::operator->()
{
	return t;
}

template<class T>
AutoGC<T>::~AutoGC()
{
	--*refCount;
	if (*refCount <= 0) {
		gcController.SetGCType(t, PendingKill);
		std::cout << "autogc lose " << "\n";
		delete refCount;
	}
	
}


class A{};
void test() {
	AutoGC<A> gc(new A[10]);
	
}