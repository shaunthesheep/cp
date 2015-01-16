#ifndef INPUT_HH
#define INPUT_HH

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <unordered_map>

#include "tinyxml2.h"

using namespace std;

struct _arc {
	int tail;
	int head;
	double length;
};

struct _node {
	_node() {
	}
	;
	_node(int t, int i, double d) {
		type = t;
		id = i;
		demand = d;
		cx = 0;
		cy = 0;
	}
	int id;
	int type;
	double demand;
	double cx;
	double cy;
};

struct _vehicle {
	int type;
	int departure_node;
	int arrival_node;
	double capacity;
};

typedef pair<int, int> arcKey;

struct arcKeyHash {
	std::size_t operator()(const arcKey& k) const {
		int k1 = k.first;
		int k2 = k.second;
		return static_cast<size_t>(1 / 2 * (k1 + k2) * (k1 + k2 + 1) + k2);
		//return std::hash< int>()(1/2*(k1 + k2)*(k1 + k2 + 1) + k2);
	}
};

struct arcKeyEqual {
	bool operator()(const arcKey& lhs, const arcKey& rhs) const {
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}
};

class Input {
	friend ostream& operator<<(ostream& os, const Input& bs);
public:
	Input(string file_name);
	~Input() {
		//delete d;
		delete [] d;
		d = NULL;
	}
	//;
	const unordered_map<arcKey, _arc, arcKeyHash, arcKeyEqual>& getArcs() const {
		return arcs;
	}

	const map<int, _node>& getNodes() const {
		return nodes;
	}
	const _node& getNode(int i) const {
		return nodes.at(i);
	}

	const vector<_vehicle>& getVehicles() const {
		return vehicles;
	}

	const _vehicle& getVehicle(int i) const {
		return vehicles[i];
	}

	const _arc& getArc(int i, int j) const {
		return arcs.at(make_pair(i, j));
	}
	//redundant
	const vector<int> getDepots() const {
		return depots;
	}
	const int getDepot(int i) const {
		return depots[i];
	}
	const char * getName() const {
		return name;
	}

	void preprocess();
	void setKUB();
	void clower();

	const int *getDistanceMatrix() const {
		return d;
	}
	const int size() const {
		return n;
	}
	const int getKUB() const {
		return K;
	}
	const int getKLB() const {
		return K_lb;
	}
	//redundant
	const vector<int> getInt2Node() const {
		return int2node;
	}
	const int getInt2Node(unsigned i) const {
		return int2node[i];
	}
	const double getTotdemand() const{
		return totdemand;
	}
	const vector<int> getDemand() const{
		return demand;
	}
protected:
	double (Input::*fdist)(int, int);
	int decimals;
	const char* name;
	bool symmetric;
	vector<int> depots;
	map<int, _node> nodes;
	unordered_map<arcKey, _arc, arcKeyHash, arcKeyEqual> arcs;
	vector<_vehicle> vehicles;
	double totdemand;
	vector<int> demand;
private:
	int n;
	int *d;
	int K_lb;
	int K;
	vector<int> int2node;
	double Euclidean(int i, int j);
};

class Output {
	friend ostream& operator<<(ostream& os, const Output& out);
	friend istream& operator>>(istream& is, Output& out);
public:
	Output(const Input& i);
	Output& operator=(const Output& out);
	void draw();
	map<int, list<int> > routes;
protected:
	const Input& in;

};

#endif
