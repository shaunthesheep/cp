#include <cstdlib>
#include <fstream>
#include <cassert>
#include <cmath>
#include <sstream>
#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <regex>
#include <climits>

#include "Data.h"
#include "tinyxml2.h"

using namespace tinyxml2;
using namespace std;

Input::Input(string file_name) {
	// Insert the code that reads the input from the file
	// and stored it into the data structures of the object

	XMLDocument doc;
	doc.LoadFile(file_name.c_str());

	if (doc.ErrorID() != 0)
		exit(EXIT_FAILURE);

	XMLElement *pRoot, *pelem;
	pRoot = doc.FirstChildElement("instance");
	for (pelem = pRoot->FirstChildElement(); pelem != NULL; pelem = pelem->NextSiblingElement()) {
		string elemName = pelem->Value();
		if (elemName == "info") {
			name = pelem->FirstChildElement("name")->GetText();
			//printf("Instance name: +%s+\n", name);
		} else if (elemName == "network") {
			XMLElement *pnodes = pelem->FirstChildElement("nodes");
			XMLElement *pnode = pnodes->FirstChildElement("node");
			while (pnode) {
				_node n;
				pnode->QueryIntAttribute("id", &n.id);
				pnode->QueryIntAttribute("type", &n.type);
				XMLElement *pc;
				if ((pc = pnode->FirstChildElement("cx"))) {
					pc->QueryDoubleText(&n.cx);
					pc = pc->NextSiblingElement("cy");
					pc->QueryDoubleText(&n.cy);
				}
				n.demand = 0;
				if (n.type == 0)
					depots.push_back(n.id);
				//printf("node: %d\n", n);
				nodes.insert(make_pair(n.id, n));
				pnode = pnode->NextSiblingElement("node");
			}

			XMLElement *plinks;
			plinks = pnodes->NextSiblingElement("euclidean");
			if (plinks) {
				plinks = pnodes->NextSiblingElement("decimals");
				fdist = &Input::Euclidean;
				plinks->QueryIntText(&decimals);
			}
			plinks = pnodes->NextSiblingElement("links");
			if (plinks == NULL) {
				for (auto i : nodes) {
					for (auto j : nodes) {
						_arc tmp_arc;
						tmp_arc.tail = i.first;
						tmp_arc.head = j.first;
						tmp_arc.length = Euclidean(i.first, j.first);
						arcs[make_pair(tmp_arc.head, tmp_arc.tail)] = tmp_arc;
					}
				}
			} else {
				plinks->QueryBoolAttribute("symmetric", &symmetric);
				XMLElement *plink = plinks->FirstChildElement("link");
				while (plink) {
					_arc tmp_arc;
					plink->QueryIntAttribute("tail", &tmp_arc.tail);
					plink->QueryIntAttribute("head", &tmp_arc.head);
					plink->FirstChildElement("length")->QueryDoubleText(&tmp_arc.length);
					//printf("link: %d %d %f\n", tail, head, length);
					arcs[make_pair(tmp_arc.tail, tmp_arc.head)] = tmp_arc;
					if (symmetric)
						arcs[make_pair(tmp_arc.head, tmp_arc.tail)] = tmp_arc;
					plink = plink->NextSiblingElement("link");
				}
			}
		} else if (elemName == "fleet") {
			XMLElement *pveic = pelem->FirstChildElement("vehicle_profile");
			while (pveic) {
				_vehicle tmp_v;
				pveic->QueryIntAttribute("type", &tmp_v.type);
				XMLElement *pdetails = pveic->FirstChildElement("departure_node");
				pdetails->QueryIntText(&tmp_v.departure_node);
				pdetails->NextSiblingElement("arrival_node")->QueryIntText(&tmp_v.arrival_node);
				pdetails->NextSiblingElement("capacity")->QueryDoubleText(&tmp_v.capacity);
				vehicles.push_back(tmp_v);
				pveic = pveic->NextSiblingElement("vehicle_profile");
				//printf("vehicle: %f\n", tmp_v.capacity);
			}

		} else if (elemName == "requests") {
			XMLElement *prequest = pelem->FirstChildElement("request");
			totdemand = 0;
			while (prequest) {
				int node;
				prequest->QueryIntAttribute("node", &node);
				prequest->FirstChildElement("quantity")->QueryDoubleText(&nodes[node].demand);
				//double* tempdem = &nodes[node].demand;
				totdemand += nodes[node].demand;
				prequest = prequest->NextSiblingElement("request");
			}
		}
	}

	//TODO: embed it better
	std::regex e(".*-k(\\d+)");
	std::cmatch cm;   // same as std::match_results<const char*> cm;
	std::regex_match(name, cm, e);
	if (cm.size() > 0) 
		K = stoi(cm[1]);
	else K =2;
	
	n = nodes.size();
	rs = n+2*K;
	int2node.reserve(n+2*K);
	int2node.resize(n);
	//for naive bin packing model
	//demand.resize(n, 0);
	//for binpacking
	demand.resize(rs, 0);
	d = new int[rs*rs];
	for(int i=0; i<rs*rs; i++)
		d[i] = 0;
	maxD = 0;
}

double Input::Euclidean(int i, int j) {
	int coeff = pow(10, decimals);
	return round(sqrt(pow(nodes[i].cx - nodes[j].cx, 2) + pow(nodes[i].cy - nodes[j].cy, 2)) * coeff)
			/ coeff;
}

void Input::clower() {
	if(!vehicles.empty())
		K_lb = ceil(getTotdemand()/vehicles[0].capacity);
	else
		K_lb = 0;
	cout << "Setting lower bound to " << K_lb << endl;
	cout << "\tTotal demand " << getTotdemand() << endl;
	cout << "\tcapacity " << vehicles[0].capacity << endl;
}

void Input::setKUB() {
// http://www.cplusplus.com/reference/regex/regex_match/
	std::regex e(".*-k(\\d+)");
	std::cmatch cm;   // same as std::match_results<const char*> cm;
	std::regex_match(name, cm, e);
	if (cm.size() > 0) {
		std::cout << "K defined as " << cm[1] << " in the instance name\n";
		K = stoi(cm[1]);
	} else {
		cout << "K not defined in the instance name.\n What should be used instead?\n";
		K = K_lb; //exit(1);
		cout << "K set to lower bound: " << K << endl;
		//K = nodes.size() - depots.size();
		//cout << "K set to upper bound (n of cities): " << nodes.size() - depots.size() << endl;
	}
}

void Input::preprocess() {
// See the section Addition for a list of things you could implement here
	demand[0] = 0;
	if(depots.size()==1){
		int i =0;
		for(auto j : nodes){
			if(j.second.type !=0 ){
				i++;
				int2node[i] = j.first;
				demand[i] = j.second.demand;
			} else {
				int2node[0] = j.first;
				demand[0] = static_cast<int>(j.second.demand); //should be 0
			}
		}	
		//starting nodes (S) and ending nodes (E) at n+1..n+2k
		vector<int> temp(2*K, int2node[0]);
		int2node.insert( int2node.end(), temp.begin(), temp.end() );
	}

	//matrix
	for(int i=0; i< n; i++){
		for(int j=0; j< n; j++){
			//distance between i and j
			if(i!=j){
				//d[i*rs + j] = static_cast<int>(Euclidean(int2node[i], int2node[j])*pow(10, decimals));
				int distance = static_cast<int>(arcs[make_pair(int2node[i],int2node[j])].length*pow(10, decimals));
				d[i*rs + j] = distance;
				maxD = std::max(maxD, distance);
			}
		}
	}
	maxD = maxD * rs;
	
	for(int j=n; j< n+2*K; j++){
		for(int i=0; i< n; i++){
			//copy first column of matrix - distances from the depot
			d[i*rs + j] = d[i*rs];
			//symmetry in the matrix
			d[j*rs + i] = d[i*rs];
		}
		d[j] = 1;
		d[j*rs] = 1;
	}

	//0 to strarting point of vehicle 1
	d[0 + n] = 1;
	d[n*rs + 0] = 1;
	//ending point of Kth vehicle to 0
	d[0 + n+2*K-1] = 1;
	d[(n+2*K-1)*rs + 0] = 1;

	for(int i=n; i< n+2*K; i++){
		for(int j=n; j< n+2*K; j++){
			if(i != j)
				d[i*rs + j] = 1;
		}
		d[(n+2*K-1)*rs + (n)] = 1;
		d[(n)*rs + (n+2*K-1)] = 1;
	}
}

ostream& operator<<(ostream& os, const Input& pa) {
	os << "Instance: " << pa.name << endl;
	os << "Nodes: " << pa.nodes.size();
	os << " Arcs: " << pa.arcs.size() << endl;
	os << "Nodes: ";
	for (auto n : pa.nodes)
		os << "[" << n.second.id << " " << n.second.type << " " << n.second.demand << "] ";
	os << endl;
	os << "Arcs: length: ";
	for (auto a : pa.arcs)
		os << a.second.tail << "-" << a.second.head << "=" << a.second.length << " ";
	os << endl;
	for (_vehicle v : pa.vehicles)
		os << "Vehicle type " << v.type << ": from " << v.departure_node << " to " << v.arrival_node
				<< " C: " << v.capacity << endl;
	os << endl;
	return os;
}

Output::Output(const Input& my_in) :
		in(my_in) {
// Insert the code that initialize the data structures of the
// output object based in the input object
	routes.clear();
	//done by setSolution() in main.cpp
}

Output& Output::operator=(const Output& out) {
// Insert the code that copies all data structures of the
// output object from the ones of the parameter out
// (excluding the reference to the input object, that is constant)
	routes.clear();
	for (auto r : out.routes) {
		routes.insert(r);
	}
	return *this;
}

ostream& operator<<(ostream& os, const Output& out) {
// Insert the code that writes the output object
	double total_length = 0;
	if(out.routes.at(0).size() > 2){
		for (auto r : out.routes) {
			os << "Route " << r.first << ": ";
			std::copy(r.second.begin(), r.second.end(), std::ostream_iterator<int>(os, " "));
			os << "\n";
			list<int>::iterator pfrom, pto;
			pto = pfrom = r.second.begin();
			pto++;
			while (pto != r.second.end()) {
				total_length += out.in.getArc(*pfrom, *pto).length;
				pfrom = pto++;
			}
		}
		cout<<"TotLength: "<<total_length<<endl;
	} else {
		if(!out.routes.empty())
			os << "K = " << out.routes.at(0).front()<< endl;
		else
			os << "K not decided yet! " << endl;
	}
	return os;
}

istream & operator>>(istream & is, Output & out) {
// Insert the code that reads the output object
	return is;
}

void Output::draw() {
	/*
	 ############################################################
	 # Creates an SVG file, tour.svg, which can be viewed with a web
	 # browser add-in (such as that at www.adobe.com\svg).  This script
	 # assumes n nodes, px[i], py[i], x[i,j], and tries to scale
	 # the graphical objects to the data, but you may still need to adjust
	 # stroke-width, circle radius, and font-size.
	 ############################################################
	 # Print SVG file header.
	 */
	ofstream os("routes.svg");
	if (!os) {
		cerr << "Cannot open output file " << endl;
		exit(1);
	}
	double max_x = 0;
	double min_x = 0;
	double max_y = 0;
	double min_y = 0;
	for (auto n : in.getNodes()) {
		if (max_x < n.second.cx)
			max_x = n.second.cx;
		if (max_y < n.second.cy)
			max_y = n.second.cy;
	}

	double tot_len = 0;

	for (auto r : routes) {
		list<int>::const_iterator ptail = r.second.cbegin();
		list<int>::const_iterator phead = next(ptail);
		while (phead != r.second.cend()) {
			double l = in.getArc(*ptail, *phead).length;
			tot_len += l;
			//cout<<*ptail<<" "<<*phead<<" "<<l<<endl;
			ptail = phead;
			phead = next(phead);
		}
	}
//	cout<<tot_len<<endl;

	double tmp = 0.005 * (max_x);
	char buf[140];

	os
			<< "<?xml version=\'1.0\' standalone=\'yes\'?>\n<!DOCTYPE svg PUBLIC \'-//W3C//DTD SVG 20010904//EN\' ";
	os << "\'http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\'>" << endl;
	os << "<svg width=\"1000\" height=\"800\" viewBox=\'";
	os << (-0.05) * max_x << " ";
	os << (-0.05) * max_y << " ";
	os << 1.3 * max_x << " ";
	os << 1.3 * max_y << " ";
	os << "\' xmlns=\'http://www.w3.org/2000/svg\'>\n<g>";
	/*
	 os << "<marker id=\"triangle\" " << " refX=\"0\" refY=\"5\" ";
	 os << " markerUnits=\"strokeWidth\" ";
	 os << " markerWidth=\"4\" markerHeight=\"3\"";
	 os << " orient=\"auto\" >\n";
	 os << " <path d=\"M 0 0 L 10 5 L 0 10 z\" />\n</marker>\n";
	 */
//	os<<"<defs>";
	os << "<marker id='triangle' orient='auto' markerWidth='2' markerHeight='4'";
	os << "       refX='0.1' refY='2'>" << endl;
	os << "   <path d='M0,0 V4 L2,2 Z' fill='red' />" << endl;
	os << "</marker>" << endl;
//  os <<"</defs>"<<endl;

// Print the length as a label.
	sprintf(buf, "<text fill=\'blue\' font-size=\'%f\' ", 0.03 * max_x);
	os << buf;

	sprintf(buf, "x=\'%f\' y=\'%f\'>Length = %f</text>\n", 1.1 * max_x, 1.1 * max_y, tot_len);
	os << buf;

// Print the nodes and their labels.
	for (auto n : in.getNodes()) {
// Print circles on the nodes.

		sprintf(buf, "<circle stroke=\'black\' stroke-width=\'%f\' cx=\'%f\' cy=\'%f\' r=\'%f\'/>\n",
				0.001 * max_x, n.second.cx, n.second.cy, 0.01 * max_x);
		os << buf;
// Print labels in the nodes.
		sprintf(buf, "<text fill=\"blue\" x=\"%f\" y=\"%f\" font-size=\"%f\">%d</text>\n",
				n.second.cx + 0.03 * max_x, n.second.cy, 0.02 * max_x, n.second.id);
		//sprintf(buf, "<text fill=\'blue\' font-size=\'%f\' x=\'%f\' y=\'%f\'>%d</text>\n",0.03 * max_x, n.second.cx, n.second.cy, n.second.id);
		os << buf;
	}
	int rbg1 = 192;
	int rbg2 = 192;
	int rbg3 = 192;
// Print the available arcs.
	for (auto a : in.getArcs()) {
		sprintf(buf, "<line stroke-width=\'%f\'", tmp);
		os << buf;
		sprintf(buf, " style =\"stroke:rgb(%d,%d,%d);stroke-width:\'%f\'\"", rbg1, rbg2, rbg3, tmp);
		os << buf;
		sprintf(buf, " stroke-dasharray=\'%f %f\'", 0.1 * tmp, tmp);
		os << buf;
		sprintf(buf, " x1=\'%f\' y1=\'%f\' x2=\'%f\' y2=\'%f\'/>\n", in.getNode(a.second.tail).cx,
				in.getNode(a.second.tail).cy, in.getNode(a.second.head).cx, in.getNode(a.second.head).cy);
		os << buf;
	}

	std::random_device rd;
//	std::uniform_int_distribution<int> dist(0, 255);
	std::uniform_int_distribution<int> dist(0, 55);
	const char *ColorValues[] = { "FF0000", "00FF00", "0000FF", "FFFF00", "FF00FF", "00FFFF",
			"000000", "800000", "008000", "000080", "808000", "800080", "008080", "808080", "C00000",
			"00C000", "0000C0", "C0C000", "C000C0", "00C0C0", "C0C0C0", "400000", "004000", "000040",
			"404000", "400040", "004040", "404040", "200000", "002000", "000020", "202000", "200020",
			"002020", "202020", "600000", "006000", "000060", "606000", "600060", "006060", "606060",
			"A00000", "00A000", "0000A0", "A0A000", "A000A0", "00A0A0", "A0A0A0", "E00000", "00E000",
			"0000E0", "E0E000", "E000E0", "00E0E0", "E0E0E0", };
	int offset = 255 / (routes.size() + 1);
	int nr = 10;		//dist(rd);
	for (auto r : routes) {
		rbg1 = offset * nr; //dist(rd);
		rbg2 = offset * nr; //dist(rd);
		rbg3 = offset * nr++; //dist(rd);
		list<int>::const_iterator ptail = r.second.cbegin();
		list<int>::const_iterator phead = next(ptail);
		while (phead != r.second.cend()) {
			sprintf(buf, "<line stroke-width=\'%f\'", tmp);
			os << buf;
			//		sprintf(buf, " style =\"stroke:rgb(%d,%d,%d);stroke-width:\'%f\'\"", rbg1, rbg2, rbg3,1.1 * tmp);
			sprintf(buf, " style =\"stroke:#%s;stroke-width:\'%f\'\"", ColorValues[nr], 1.1 * tmp);
			os << buf;
			os << " marker-end=\"url(#triangle)\" marker-mid=\"url(#triangle)\" ";
			sprintf(buf, " x1=\'%f\' y1=\'%f\' x2=\'%f\' y2=\'%f\'/>\n", in.getNode(*ptail).cx,
					in.getNode(*ptail).cy, in.getNode(*phead).cx, in.getNode(*phead).cy);
			os << buf;
			ptail = phead;
			phead = next(phead);
		}
		nr = (++nr) % 56;
	}

// Print the end of the "tour.svg", and close it.
	os << "</g></svg>";

	cout << "Written file routes.svg." << endl;

}
