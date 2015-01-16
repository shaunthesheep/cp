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

class VRPSolver: public Script {
protected:
	Input *p_in;
	int model;

public:
	/// Model variants
	enum {
		MODEL_TASK3, ///< K_LB
		MODEL_TASK4, ///< VRP
	};

	VRPSolver(const InstanceOptions& opt, Input *in) :
			p_in(in) {

		int _n = p_in->size();
		int _k = p_in->getKUB();

		model = opt.model();

		if (opt.model() == MODEL_TASK3) {
			// your model
			// branch on something
		}
		if (opt.model() == MODEL_TASK4) {
			// Your model
			// branch on something
		}

	}
	;

/*
/// Setup model
	VRPSolver(const InstanceOptions& opt, Input *in) :
			p_in(in) { //, 
			//load(*this, p_in->getKUB(), 0, static_cast<int>(p_in->getVehicles()[0].capacity)), 
			//vehicle(*this, p_in->size(), 0, p_in->getKUB()-1),
			//vehicles(*this, p_in->getKLB(), p_in->getKUB()) {
			
			//load(*this, _k, 0, _c), vehicle(*this, _n, 0, _k-1) {

		//int _n = p_in->size();			//number of nodes
		//int _k = p_in->getKUB();		//upper bound
		//p_in->preprocess();


		model = opt.model();

		if (opt.model() == MODEL_TASK3) {

			// your model
			int _l = p_in->getKLB();		//naive lower bound
			int _c = static_cast<int>(p_in->getVehicles()[0].capacity);
			int _s = static_cast<int>(p_in->getTotdemand());

			load = IntVarArray(*this, _k, 0, _c);
			vehicle = IntVarArray(*this, _n, 0, _k-1);
			vehicles = IntVar(*this, _l, _k);

			//excess vehicles
			for(int i=0; i<_k; i++)
				rel(*this, (vehicles<i+1)==(load[i]==0));

			////bin packing
			//assert(p_in->getDemand().size() == p_in->size());
			//IntArgs sizes(p_in->getDemand());
    		//binpacking(*this, load, vehicle, sizes);

    		////naive bin packing
    		int s=0;
    		int size[_n] ;// = new int[_n];
		    for (int i=0; i<_n; i++){
		      s += p_in->getDemand()[i];
		      size[i] = p_in->getDemand()[i];
		  	}
		    IntArgs sizes(_n, size);
		    // loads add up to item sizes
		    linear(*this, load, IRT_EQ, s);
		    // loads are equal to packed items
		    BoolVarArgs _x(*this, _n*_k, 0, 1);
		    Matrix<BoolVarArgs> x(_x, _n, _k);
		    for (int i=0; i<_n; i++)
		      channel(*this, x.col(i), vehicle[i]);
		    for (int j=0; j<_k; j++)
		    	linear(*this, sizes, x.row(j), IRT_EQ, load[j]);


    		// symmetry breaking
		    for (int i=1; i<_n; i++)
		      if (p_in->getDemand()[i-1] == p_in->getDemand()[i])
		        rel(*this, vehicle[i-1] <= vehicle[i]);

		    // pack items that require a vehicle
		    for (int i=0; (i < _n) && (i < _k) && (p_in->getDemand()[i] * 2 > _c); i++)
		      rel(*this, vehicle[i] == i);

			// branch on something
			branch(*this, vehicles, INT_VAL_MIN());
    		branch(*this, vehicle, INT_VAR_NONE(), INT_VAL_MIN());

		}
		if (opt.model() == MODEL_TASK4) {
			// Your model
			// branch on something
		}

	}
	;

*/

	virtual void constrain(const Space& _b) {
		// implement this or a cost function
	}
	;
	/*
	virtual IntVar cost(void) const {
		if (opt.model() == MODEL_TASK3) {
    		return vehicles;
    	}
    	if (opt.model() == MODEL_TASK4) {
    		return 0;
    	}
  	}*/

/// Constructor for cloning s
	VRPSolver(bool share, VRPSolver& s) :
			Script(share, s), p_in(s.p_in) {
		// remember to update your main variables!
		model = s.model;
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
	/*
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
	*/
		}
		if (model == MODEL_TASK4) {
		}
	}
	;

	virtual void setSolution(Output &o) {

		// write in o.routes your solution
		// remember that the first and last element of each route list
		// must be the depot
		// clear first:
		// o.routes.clear();
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
	opt.instance("data/augerat-r/P/P-n016-k08.xml");

	opt.time(6 * 1000); // in milliseconds

	opt.parse(argc, argv);

	Input *p = new Input(opt.instance());
	p->clower();
	//cout << *p;
	p->setKUB();
	p->preprocess();
	cout << *p;

	cout << "Time limit: " << opt.time() / 1000 << "s" << endl;
	cout << "Threads: " << opt.threads() << endl;

	// This is to stop the search at the time limit imposed
	Search::Options so;
	Search::TimeStop* ts;
	ts = new Search::TimeStop(opt.time());
	so.stop = ts;

	// For Restarts
	Search::Cutoff* c;
	switch (opt.restart()) {
	case RM_NONE:
		c = Search::Cutoff::constant(std::numeric_limits<unsigned long>::max());
		break;
	case RM_LUBY:
		c = Search::Cutoff::luby(opt.restart_scale());
		break;
		// add here other cases
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

	cout << "HERE" << std::endl;
	VRPSolver* m = new VRPSolver(opt, p);
	cout << "HERE" << std::endl;
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
		DFS<VRPSolver> e(m, so);
		//BAB<VRPSolver> e(m, so);
		//RBS<BAB, VRPSolver> e(m, so);
		delete m;
		while (VRPSolver* s = e.next()) {
			s->print(cout);
			s->setSolution(out); // pass here the solution found to Output for drawing
			out.draw();
			delete s;
			stat = e.statistics();
			//cout << "Propagators: " << stat.propagate << endl; // see page 145 MPG
			print_stats(stat);
			cout << "\ttime: " << t.stop() / 1000 << "s" << endl;

			//break;
		}
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

	//delete p->getDistanceMatrix();
	delete p;
	return 0;
}

