/*
 * main.cpp
 *
 *  Created on: Nov 13, 2014
 *      Author: marco
 *		Edited by: anna
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <algorithm>
#include <iomanip>
#include <limits>

#include "Data.h"

using namespace Gecode;

class VRPSolver: public Space  {
protected:
	Input *p_in;
	int model;
	IntVarArray load;				//loads in each vehicle
	IntVarArray vehicle;			//vehicle numbers assigned to nodes
	IntVarArray succ;
	IntVar vehicles;				//cost of bin packing
	IntVar total;					//cost of tsp

public:
	/// Model variants
	enum {
		MODEL_TASK3, ///< K_LB
		MODEL_TASK4, ///< VRP
	};

	VRPSolver(const InstanceOptions& opt, Input *in) :
			p_in(in),
			load(*this, p_in->getKUB(), 0, static_cast<int>(p_in->getVehicles()[0].capacity)), 
			vehicle(*this, p_in->getRs(), 0, p_in->getKUB()-1),
			vehicles(*this, p_in->getKLB(), p_in->getKUB()),
			succ(*this, p_in->getRs(), 0, p_in->getRs()-1),
			total(*this, 0, p_in->getMaxD()){

		int _l = p_in->getKLB();
		int _n = p_in->size();
		int _k = p_in->getKUB();
		int rs = p_in->getRs();
		int _c = static_cast<int>(p_in->getVehicles()[0].capacity);

		model = opt.model();

		if (opt.model() == MODEL_TASK3) {

			// your model
			IntArgs sizes(p_in->getDemand());
			binpacking(*this, load, vehicle, sizes);

/*		//naive bin packing

			IntArgs sizes(p_in->getDemand());
			
			int s = p_in->getTotdemand();
		    // loads add up to item sizes
		    linear(*this, load, IRT_EQ, s);
		    // loads are equal to packed items
		
		    BoolVarArgs _x(*this, _n*_k, 0, 1);
		    Matrix<BoolVarArgs> x(_x, _n, _k);
		    for (int i=0; i<_n; i++)
		      channel(*this, x.col(i), vehicle[i]);
		    for (int j=0; j<_k; j++)
		    	linear(*this, sizes, x.row(j), IRT_EQ, load[j]);
*/
			// All excess vehicle must be empty
			for (int j=_l+1; j <= _k; j++)
				rel(*this, (vehicles < j) == (load[j-1] == 0));

			// pack items that require a vehicle
		    for (int i=0; (i < _n) && (i < _k) && (p_in->getDemand()[i] * 2 > _c); i++)
		      rel(*this, vehicle[i] == i);

			// Break symmetries
			for (int i=1; i<_n; i++)
				if (p_in->getDemand(i-1) == p_in->getDemand(i))
					rel(*this, vehicle[i-1] <= vehicle[i]);
				
			// branch on something
			branch(*this, vehicle, INT_VAR_MAX_MAX(), INT_VAL_MAX());
			//branch(*this, vehicle, INT_VAR_NONE(), INT_VAL_MIN());
			branch(*this, vehicles, INT_VAL_MIN());

		}
		if (opt.model() == MODEL_TASK4) {

			//number of vehicles given
			dom(*this, vehicles, _k);

			////bin packing
			IntArgs sizes(p_in->getDemand());
			binpacking(*this, load, vehicle, sizes);
			// pack items that require a vehicle
		    for (int i=0; (i < _n) && (i < _k) && (p_in->getDemand()[i] * 2 > _c); i++)
		      rel(*this, vehicle[i] == i);

			//assign starting nodes to vehicles
			dom(*this, vehicle[0], 0);
			for(int i=0; i<_k; i++){
				//assign starting nodes to vehicles
				dom(*this, vehicle[_n+i], i);
				//assign ending nodes to vehicles
				dom(*this, vehicle[_n+_k+i], i);
//?				//breaking symmetry within each route
				for(int j=1; j<_n; j++){
					BoolVar bs(*this, 0, 1);
					BoolVar be(*this, 0, 1);
					rel(*this, succ[j] , IRT_EQ, (_n+_k+i) , be);
					rel(*this, succ[_n+i] , IRT_LE, j, bs);
					rel(*this, be, BOT_IMP,  bs, 1);
				}
			}

			//new vehicle starts where old finished
			for(int i=0; i<_k-1; i++)
				dom(*this, succ[_n+_k+i], _n+(i+1));
			dom(*this, succ[_n+2*_k-1], 0);
			dom(*this, succ[0], _n);
			//the successor of a nonending node is served by the same vehicle
			for(int i=0; i<_n+_k; i++)
				element(*this, vehicle, succ[i], vehicle[i]);
//*/
			//cost matrix
			IntArgs c (rs*rs, p_in->getDistanceMatrix());

			//zero as infinity - no loops
			for (int i=0; i<rs; i++)	
		      for (int j=0; j<rs; j++)	
		        if (p_in->dist(i,j) == 0)
		          rel(*this, succ[i], IRT_NQ, j);

		    //distinct(*this, succ);

		    

		    // Cost of each edge
			IntVarArgs costs(*this, rs, Int::Limits::min, Int::Limits::max);
			// Enforce that the succesors yield a tour with appropriate costs
			circuit(*this, c, succ, costs, total, opt.icl());

			// branch on something
			//* //BASIC
			//branch(*this, vehicle, INT_VAR_NONE(), INT_VAL_MIN());
			branch(*this, vehicle, INT_VAR_MAX_MAX(), INT_VAL_MAX());

			// First enumerate cost values, prefer those that maximize cost reduction
			branch(*this, costs, INT_VAR_REGRET_MAX_MAX(), INT_VAL_SPLIT_MIN());

			// Then fix the remaining successors
			branch(*this, succ,  INT_VAR_MIN_MIN(), INT_VAL_MIN());
			branch(*this, vehicles, INT_VAL_MIN());

			/* //BLUE
			//branch(*this, vehicle, INT_VAR_NONE(), INT_VAL_MIN());
			branch(*this, vehicle, INT_VAR_SIZE_MIN(), INT_VAL_MAX());

			// First enumerate cost values, prefer those that maximize cost reduction
			branch(*this, costs, INT_VAR_REGRET_MAX_MAX(), INT_VAL_SPLIT_MAX());

			// Then fix the remaining successors
			branch(*this, succ,  INT_VAR_MIN_MIN(), INT_VAL_MIN());
//*/

			/* //ORANGE - RECORDED
			branch(*this, vehicle, INT_VAR_MAX_MAX(), INT_VAL_MAX());

			branch(*this, costs, INT_VAR_REGRET_MAX_MAX(), INT_VAL_SPLIT_MAX());

			branch(*this, succ,  INT_VAR_MIN_MIN(), INT_VAL_MIN());
//*/
		}

	}
	;

	virtual void constrain(const Space& _b) {
		const VRPSolver& b = static_cast<const VRPSolver&>(_b);
		if (model == MODEL_TASK3) {
			IntVar b_v(b.vehicles);
    		rel(*this, vehicles, IRT_LE, b_v);
    	}
    	if (model == MODEL_TASK4) {
    		IntVar b_t(b.total);
    		rel(*this, total, IRT_LE, b_t);
    	}
	}
	;

/// Constructor for cloning s
	VRPSolver(bool share, VRPSolver& s) :
			Space(share, s), p_in(s.p_in) {
		// remember to update your main variables!
		model = s.model;
		load.update(*this, share, s.load);
    	vehicle.update(*this, share, s.vehicle);
    	vehicles.update(*this, share, s.vehicles);
		succ.update(*this, share, s.succ);
    	total.update(*this, share, s.total);
	}
	;
/// Copy during cloning
	virtual Space* copy(bool share) {
		return new VRPSolver(share, *this);
	}
	;
/// Print solution
	virtual void print(std::ostream& os) const {
		if (model == MODEL_TASK3) {
			os << "Vehicles used: " << vehicles << " (from " << p_in->getKLB() << " and "<< p_in->getKUB() << " vehicles)." << std::endl;
		    os << "Demand: " << p_in->getDemand()[0] <<std::endl;
		    
		    for (int j=0; j<p_in->getKUB(); j++) {
		      bool fst = true;
		      os << "\t[" << j << "]={";
		      for (int i=0; i<p_in->size(); i++)
		        if (vehicle[i].assigned() && (vehicle[i].val() == j)) {
		          if (fst) {
		            fst = false;
		          } else {
		            os << ",";
		          }
		          os << i;
		        }
		      os << "} #" << load[j] << std::endl;
		    }
		    if (!vehicle.assigned()) {
		      os << std::endl 
		         << "Unserved demands:" << std::endl;
		      for (int i=0;i<p_in->size(); i++)
		        if (!vehicle[i].assigned())
		          os << "\t[" << i << "] = " << vehicle[i] << std::endl;
		    }
		}
		if (model == MODEL_TASK4) {
			bool assigned = true;
			for (int i=0; i<succ.size(); i++) {
				if (!succ[i].assigned()) {
					assigned = false;
					break;
				}
			}
			if (assigned) {
				os << "\tTour: ";
				int i=0;
				do {
					os << i << " -> ";
					i=succ[i].val();
				} while (i != 0);
				os << 0 << std::endl;
				os << "\tCost: " << total << std::endl;
			} else {
				os << "\tTour: " << std::endl;
				for (int i=0; i<succ.size(); i++) {
					os << "\t" << i << " -> " << succ[i] << std::endl;
				}
				os << "\tCost: " << total << std::endl;
			}

		}
	}
	;

	virtual void setSolution(Output &o) {

		// write in o.routes your solution
		// remember that the first and last element of each route list
		// must be the depot
		// clear first:
		o.routes.clear();
		if (model == MODEL_TASK3) {
			vector<int> veh;
			if(vehicles.assigned()){
				veh.push_back(vehicles.val());
				std::list<int> vehK;
				std::copy( veh.begin(), veh.end(), std::back_inserter( vehK ) );
				o.routes[0] = vehK;
			}
		}
		if (model == MODEL_TASK4) {
			int _n = p_in->size();
			int _k = p_in->getKUB();
			for(int i=0; i<_k; i++){
				vector<int> route;
				route.push_back(p_in->getInt2Node(0));
				if(succ[_n+i].assigned()){
					int next = succ[_n+i].val();
					while(true){
						if(next==_n+_k+i)
							break;
						route.push_back(p_in->getInt2Node(next));
						if(succ[next].assigned())
							next = succ[next].val();
						else{
							route.clear();
							route.push_back(p_in->getInt2Node(0));
							break;
						}
					}
				}

				route.push_back(p_in->getInt2Node(0));
				std::list<int> routel;
				std::copy( route.begin(), route.end(), std::back_inserter( routel ) );
				o.routes[i] = routel;
			}
		}
	}
	;
};

void print_stats(Search::Statistics &stat) {
	cout << "\tfail: " << stat.fail << endl;
	cout << "\tnodes: " << stat.node << endl;
	cout << "\tpropagators: " << stat.propagate << endl; // see page 145 MPG
	cout << "\tdepth: " << stat.depth << endl;
	cout << "\trestarts: " << stat.restart << endl;
	cout << "\tnogoods: " << stat.nogood << endl;
}

int main(int argc, char* argv[]) {

	InstanceOptions opt("VRPSolver");
	opt.model(VRPSolver::MODEL_TASK4);
	opt.model(VRPSolver::MODEL_TASK3, "TASK3", "Find a lower bound to K");
	opt.model(VRPSolver::MODEL_TASK4, "TASK4", "Find min tot length");
	opt.instance("../data/augerat-r/P/P-n051-k10.xml");

	opt.time(6 * 1000); // in milliseconds

	opt.parse(argc, argv);

	Input *p = new Input(opt.instance());
	p->clower();
	//cout << *p;
	p->setKUB();
	p->preprocess();
/*
	cout << "demands:  " << p->getDemand().size() << std::endl;
	cout << *p;

	cout << "Time limit: " << opt.time() / 1000 << "s" << endl;
	cout << "Threads: " << opt.threads() << endl;
*/
	// This is to stop the search at the time limit imposed
	Search::Options so;
	Search::TimeStop* ts;
	ts = new Search::TimeStop(opt.time());
	so.stop = ts;

	// For Restarts
	Search::Cutoff* c;
	int b;
	switch (opt.restart()) {
	case RM_NONE: //No restarts.
		c = Search::Cutoff::constant(std::numeric_limits<unsigned long>::max());
		break;
	case RM_LUBY: //Restart with Luby sequence. 1,1,2,1,1,2,4,1,1,2,1,1,2,4,8,...
		c = Search::Cutoff::luby(opt.restart_scale());
		break;
	case RM_CONSTANT: //Restart with constant sequence.
		c = Search::Cutoff::constant(opt.restart_scale());
		break;
	case RM_GEOMETRIC: //Restart with geometric sequence.
		c = Search::Cutoff::geometric(opt.restart_scale(),b);   // b???
		break;
	case RM_LINEAR: //Restart with linear sequence.
		c = Search::Cutoff::linear(opt.restart_scale());
		break;
	}
	so.cutoff = c;
	cout << "Restarts: " << opt.restart() << endl;

	// No Goods: Should they be used or not?
	if (opt.nogoods())
		so.nogoods_limit = opt.nogoods_limit();
	cout << "NoGood Limit: " << so.nogoods_limit << endl;

	Search::Statistics stat;
	// Let's start the Timer
	Support::Timer t;
	t.start();
	double elapsed;

/*
	for(int i=0; i<p->getDemand().size(); i++)
			cout << "Demands: " << p->getDemand()[i] << " \tint2node " << p->getInt2Node()[i]<<std::endl;
		cout << "Total demand: " << p->getTotdemand() <<std::endl;
		cout<< "Size of instance - customers rs: " << p->getRs() << std::endl;
*/
	VRPSolver* m = new VRPSolver(opt, p);
	SpaceStatus status = m->status();
	m->print(cout);
	if (status == SS_FAILED)
		cout << "Status: " << m->status() << " the space is failed at root." << endl;
	else if (status == SS_SOLVED)
		cout << "Status: " << m->status()
				<< " the space is not failed but the space has no brancher left." << endl;
	else if (status == SS_BRANCH)
		cout << "Status: " << m->status() << " the space is not failed and we need to start branching."
				<< endl;

	Output out(*p);

	try {
		//DFS<VRPSolver> e(m, so);
		BAB<VRPSolver> e(m, so);
		//RBS<BAB, VRPSolver> e(m, so);
		delete m;
		int currentBest = p->getMaxD();
		while (VRPSolver* s = e.next()) {
			//s->print(cout);
			s->setSolution(out); // pass here the solution found to Output for drawing
			//out.draw();
			delete s;
			stat = e.statistics();
			////cout << "Propagators: " << stat.propagate << endl; // see page 145 MPG
			//print_stats(stat);
			//cout << "\ttime: " << t.stop() / 1000 << "s" << endl;
			////break;
			s->print(cout);
		}
		out.draw();

		if (e.stopped()) {
			cout << "WARNING: solver stopped, solution is not optimal!\n";
			if (ts->stop(e.statistics(), so)) {
				cout << "\t Solver stopped becuase of TIME LIMIT!\n";
			}
		}
		print_stats(stat);
		cout << "\ttime: " << t.stop() / 1000 << "s" << endl;
		cout << out << endl;
	} catch (Exception e) {
		std::cerr << "Gecode exception: " << e.what() << std::endl;
		return 1;
	}

	delete p;
	return 0;
}

