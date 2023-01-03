#pragma once


#ifndef KD_TREE_H
#define KD_TREE_H
#include"AccelStructure.hpp"
#include"bound.hpp"
#include"intersection.hpp"
#include"ray.hpp"
#include"shape.hpp"
#include<vector>
#include<memory>
#include<algorithm>

const static float EPS = 0.0001;

struct KdTreeNode
{
public:
	union 
	{
		float split;
		int onePrimitive;
		int* primitives;
	};
	union 
	{
		int flag;
		int nPrims;
		int aboveChild;
	};
	void initLeaf(int* prims, unsigned n) {
		flag = 3;
		nPrims |= (n << 3);
		if (n == 0)
			onePrimitive = -1;
		else if (n == 1)
			onePrimitive = prims[0];
		else {
			primitives = new int[n];
			for (unsigned i = 0; i < n; ++i) {
				//std::cout << "pr:" << prims[i] << "\n";
				primitives[i] = prims[i];
			}
		}
	}

	void initInterior(int axis, float sp, int ac) {
		split = sp;
		flag = axis;
		aboveChild |= (ac << 2);
	}

	bool isLeaf() const { return (flag & 3) == 3; }
	float SplitPos() const { return split; }
	int SplitAxix() const { return (flag & 3); }
	int nPrimitives() const { return (nPrims >> 2); }
	int AboveChild() const { return (aboveChild >> 2); }
};

enum class EdgeType {  Start=0, End=1};

struct BoundEdge
{
public:
	float t;
	int prim;
	EdgeType type;
	BoundEdge() = default;
	BoundEdge(float tt, int p, bool starting) :t(tt), prim(p){
		type = starting ? EdgeType::Start : EdgeType::End;
	}

	BoundEdge& operator=(const BoundEdge& b) {
		t = b.t;
		prim = b.prim;
		type = b.type;
		return *this;
	}
};

struct KdToDo {
	KdTreeNode* node;
	float tMin, tMax;
};

class KdTree : public AccelStructrue{
private:
	std::vector<Shape*> primitives;
	int icost;
	int tcost;
	float emptyBonus;
	int maxDepth;
	int maxPrims;
	KdTreeNode* nodes;
	int nAllocNodes, nextFreeNode;
	Bounds3 box;

public:
	KdTree(std::vector<Shape*>& prims, int ic=80, int tc=1, float eb=0.5f) :
		primitives(std::move(prims)), icost(ic), tcost(tc), emptyBonus(eb) {
		int nPrim = primitives.size();
		maxDepth = int(8 + 1.3f * std::log2f(float(nPrim)));
		std::cout <<"\n"<<nPrim<< "  maxdepth: " << maxDepth << "\n";
		maxPrims = nPrim + 1;
		nAllocNodes = nextFreeNode = 0;

		std::vector<Bounds3> allBounds;
		for (unsigned i = 0; i < nPrim; ++i) {
			auto b = primitives[i]->getBounds();
			if (i > 0)
				box = Union(box, b);
			else
				box = b;
			allBounds.push_back(std::move(b));
		}
		std::cout << box.pMin.x<<"\n";
		std::unique_ptr<BoundEdge[]> edges[3];
		for (int i = 0; i < 3; ++i)
			edges[i].reset(new BoundEdge[2 * nPrim]);
		std::unique_ptr<int[]> prims0(new int[nPrim]);
		std::unique_ptr<int[]> prims1(new int[(maxDepth + 1) * nPrim]);

		std::unique_ptr<int[]> primNums(new int[nPrim]);
		for (int i = 0; i < nPrim; ++i) primNums[i] = i;

		build(0, box, allBounds, primNums.get(), nPrim, maxDepth, edges, prims0.get(), prims1.get());
		std::cout << "depth " << nextFreeNode<<"\n";
	}

	void build(int nodeNum, const  Bounds3& bound, const std::vector<Bounds3>& allBounds
		, int* primNums, int nPrimitives, int depth, std::unique_ptr<BoundEdge[]> edges[3], 
		int* prims0, int* prims1) {
		//std::cout << depth <<" "<<nPrimitives<< "\n";
		if (nAllocNodes <= nextFreeNode) {
			int nAlloc = std::max(nAllocNodes * 2, 512);
			KdTreeNode* n = new KdTreeNode[nAlloc];
			if (nAllocNodes > 0) {
				memcpy(n, nodes, sizeof(KdTreeNode) * nAllocNodes);
				delete nodes;
			}
			nAllocNodes = nAlloc;
			nodes = n;
		}
		++nextFreeNode;

		if (nPrimitives <= 16 || depth == 0) {
			nodes[nodeNum].initLeaf(primNums, nPrimitives);
			if (depth == 0) {
				//std::cout << "nPrs: " << nPrimitives << "\n";
			}
			return;
		}

		int bestAxis = -1; int bestOffset = -1;
		float bestCost = 0x777fffff;
		float totalSA = bound.SurfaceArea();
		Vector3f d = bound.Diagonal();
		float invTotleSA = 1.f / totalSA;
		int axis = bound.maxExtent();
		for (int i = 0; i < 3 && bestAxis==-1; i++) {
			axis += i;
			axis %= 3;
			for (int j = 0; j < nPrimitives; ++j) {
				auto b = allBounds[primNums[j]];
				edges[axis][2*j] = BoundEdge(b.pMin[axis], primNums[j], true);
				edges[axis][2*j+1] = BoundEdge(b.pMax[axis], primNums[j], false);
			}
			
			std::sort(&edges[axis][0],&edges[axis][2*nPrimitives], 
				[](const BoundEdge& e0, const BoundEdge& e1) -> bool {
				if (e0.t == e1.t)
					return (int)e0.type < (int)e1.type;
				else
					return e0.t < e1.t;
				});
			

			int nBelow = 0; int nAbove = nPrimitives;
			for (int j = 0; j < 2 * nPrimitives; ++j) {
				if (edges[axis][j].type == EdgeType::End) --nAbove;
				float split = edges[axis][j].t;
				if (split > bound.pMin[axis]+EPS && split < bound.pMax[axis]-EPS) {
					int a1 = (axis + 1) % 3;
					int a2 = (axis + 2) % 3;
					float belowSA = 2 * (d[a1] * d[a2] + (split - bound.pMin[axis])*( d[a1] + d[a2]));
					float aboveSA = 2 * (d[a1] * d[a2] + (bound.pMax[axis] - split) * (d[a1] + d[a2]));
					float pB = belowSA * invTotleSA;
					float pA = aboveSA * invTotleSA;
					float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;
					float cost = tcost + icost * (1.f - eb) * (pB * nBelow + pA * nAbove);
					if (cost < bestCost) {
						bestCost = cost;
						bestAxis = axis;
						bestOffset = j;
					}
				}
				if (edges[axis][j].type == EdgeType::Start) ++nBelow;
			}

			
		}

		if (bestAxis == -1 || bestOffset == -1) {
			nodes[nodeNum].initLeaf(primNums, nPrimitives);
			return;
		}

		int n0 = 0, n1 = 0;
		for (int j = 0; j < bestOffset; ++j)
			if (edges[bestAxis][j].type == EdgeType::Start)
				prims0[n0++] = edges[bestAxis][j].prim;
		for (int j = bestOffset + 1; j < 2 * nPrimitives; ++j)
			if (edges[bestAxis][j].type == EdgeType::End)
				prims1[n1++] = edges[bestAxis][j].prim;

		float tsplit = edges[bestAxis][bestOffset].t;
		Bounds3 bounds0 = bound, bounds1 = bound;
		bounds0.pMax[bestAxis] = bounds1.pMin[bestAxis] = tsplit;
		build(nodeNum + 1, bounds0,
			allBounds, prims0, n0, depth - 1, edges,
			prims0, prims1 + nPrimitives);
		uint32_t aboveChild = nextFreeNode;
		nodes[nodeNum].initInterior(bestAxis, aboveChild, tsplit);
		build(aboveChild, bounds1, allBounds, prims1, n1,
			depth - 1, edges, prims0, prims1 + nPrimitives);
	}

	bool intersect(const Ray& r, Intersection* inter) {
		float tMin, tMax;
		if (!box.Intersect(r, &tMin, &tMax))
			return false;

		KdTreeNode* node = &nodes[0];
		KdToDo KdToDoList[128];
		Vector3f invDir = r.direction_inv;
		unsigned toDoPos = 0;
		bool hit = false;

		while (node) {
			if (r.t_max < tMin||hit);
				break;
			if (!node->isLeaf()) {
				int axis = node->SplitAxix();
				float tPlane = (node->SplitPos() - r.origin[axis]);

				KdTreeNode* first, *second;
				int belowFirst = r.origin[axis] < node->SplitPos() ||
					(r.origin[axis] == node->SplitPos() && r.direction[axis] <= 0);

				if (belowFirst) {
					first = node + 1;
					second = &nodes[node->AboveChild()];
				}
				else {
					first = &nodes[node->AboveChild()];
					second = node + 1;
				}

				if (tPlane > tMax || tPlane <= 0)
					node = first;
				else if (tPlane < tMin)
					node = second;
				else {
					KdToDoList[toDoPos].node = second;
					KdToDoList[toDoPos].tMin = tPlane;
					KdToDoList[toDoPos].tMax = tMax;
					++toDoPos;
					node = first;
					tMax = tPlane;
				}
			}

			else {
				int nPrims = node->nPrimitives();
				if (nPrims == 1) {
					auto prim = primitives[node->onePrimitive];
					*inter = prim->getIntersection(r);
					hit = inter->happened;
				}
				else if (nPrims > 1) {
					for (int i = 0; i < nPrims; ++i) {
						auto prim = primitives[node->primitives[i]];
						*inter = prim->getIntersection(r);
						hit = inter->happened;
					}
				}

				if (toDoPos > 0) {
					--toDoPos;
					node = KdToDoList[toDoPos].node;
					tMin = KdToDoList[toDoPos].tMin;
					tMax = KdToDoList[toDoPos].tMax;
				}
				else
					break;
			}

		}

		if (hit)
			std::cout << "\nhit\n";
		return hit;
	}

	Bounds3 GetBounds() const override {
		return box;
	}
};
#endif // !KD_TREE_H
