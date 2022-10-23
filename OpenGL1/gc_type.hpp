
#ifndef GC_TYPE_H
#define GC_TYPE_H

using gc_level = unsigned char;
#define GC_LEVEL0 0
#define GC_LEVEL1 1
#define GC_LEVEL2 2

enum GCType
{
	Live,PendingKill,Die
};



#endif // !GC_TYPE_H
