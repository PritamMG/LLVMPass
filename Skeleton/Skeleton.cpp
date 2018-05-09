#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CommandLine.h"
#include<set>
#include<vector>
#include<list>
#include<map>
#include<string.h>

using namespace llvm;

using namespace std;

typedef CallGraphNode* CGN;
typedef set< unsigned int > set_of_ints;
typedef set_of_ints::iterator iter;

// For Command Line Arguments

static cl::opt<string> caller_name("caller", cl::desc("Specify Caller"), cl::Required);
static cl::opt<string> callee_name("callee", cl::desc("Specify Callee"), cl::Required);

namespace {
    struct SkeletonPass : public ModulePass {
        static char ID;
        SkeletonPass() : ModulePass(ID) { }

        virtual bool runOnModule(Module &M) override;

//	Customized Data Structures	

	static unsigned int CGN_COUNT;	

// 	Mapping CallGraphNode with a unique identifier

	map< unsigned int, CGN > map_call_graph_node; 
	map< CGN, unsigned int > rev_map_call_graph_node;

	set_of_ints already_visited;

//	Maps for storing call graph information
	
	map< unsigned int, set_of_ints > caller_rel;
	map< unsigned int, set_of_ints > callee_rel;

	map< unsigned int, map< unsigned int, bool > > caller_callee_rel;

	void print_call_graph();

	bool checkForCallerCalleeRel(unsigned int caller, unsigned int callee);
	bool checkForCallerCalleeInProgram(StringRef caller, StringRef callee);

	void checkForCallerCalleeRelForAll();
    };
}
char SkeletonPass::ID = 0;

unsigned int SkeletonPass::CGN_COUNT = 1;

void SkeletonPass::print_call_graph()
{
	for(map< unsigned int, set_of_ints >::iterator it = callee_rel.begin(); it != callee_rel.end(); it++)
	{
		errs() << "Function " << map_call_graph_node[it->first]->getFunction()->getName() << ": Callees\n";

		for(iter cit = it->second.begin(); cit != it->second.end(); cit++) 
		{
			errs() << map_call_graph_node[*cit]->getFunction()->getName() << "\n";
		}
	}
}

bool SkeletonPass::checkForCallerCalleeRel(unsigned int caller, unsigned int callee)
{
	set_of_ints callees, new_callers;	

//	errs() << "Caller " << caller << " Callee " << callee << "\n";

	callees = callee_rel[caller];

	if(callees.empty())
	{
		return false;
	}

//	bool ret = true;

	already_visited.insert(caller);

	for(iter it = callees.begin(); it != callees.end(); it++)
	{
//		errs() << "Callee " << *it << "\n";

		if(*it == callee)
		{
//			errs() << "Callee " << *it << " returning true \n";

			return true;
		}
		else
		{
			if(already_visited.find(*it) == already_visited.end())
			{
				new_callers.insert(*it);
//				return checkForCallerCalleeRel(*it, callee);				
			}
		}
	}

	for(iter cit = new_callers.begin(); cit != new_callers.end(); cit++)
	{
		return checkForCallerCalleeRel(*cit, callee);
	}

	return false;
}

bool SkeletonPass::checkForCallerCalleeInProgram(StringRef caller, StringRef callee)
//bool checkForCallerCalleeInProgram(char * caller, char * callee)
{
	unsigned int caller_id = 0, callee_id = 0;

	for(map< CGN, unsigned int >::iterator it = rev_map_call_graph_node.begin(); it != rev_map_call_graph_node.end(); it++)
	{
		if(caller.equals(it->first->getFunction()->getName()))
//		if (strcmp (it->first->getFunction()->getName(), caller) == 0)
		{
			caller_id = it->second;
		}

		if(callee.equals(it->first->getFunction()->getName()))
//		if (strcmp (it->first->getFunction()->getName(), callee) == 0)
		{
			callee_id = it->second;
		}

		if(caller_id != 0 && callee_id != 0)
		{
			break;
		}
	}

	if(caller_id == 0)
	{
		errs() << "Caller " << caller << " Not Found\n";
		exit(1);
	}

	if(callee_id == 0)
	{
		errs() << "Callee " << callee << " Not Found\n";
		exit(1);
	}

//	errs() << "Caller: " << caller << " Found with id: " << caller_id <<  "\n";
//	errs() << "Callee: " << callee << " Found with id: " << callee_id <<  "\n";

	already_visited.clear();

//	if(caller_callee_rel[caller_id][callee_id])
	if(checkForCallerCalleeRel(caller_id, callee_id))
	{
		errs() << "Caller " << caller << " -> Callee " << callee << " => True\n";
		return true;
	}	
	else 
	{
		errs() << "Caller " << caller << " -> Callee " << callee << " => False\n";
		return false;
	}
}

void SkeletonPass::checkForCallerCalleeRelForAll()
{
	for(unsigned int i = 1; i <= CGN_COUNT; i++)
	{
		already_visited.clear();

		for(unsigned int j = 1; j <= CGN_COUNT; j++)
		{
			caller_callee_rel[i][j] = checkForCallerCalleeRel(i, j);
		}
	}
}

bool SkeletonPass::runOnModule(Module &M) {

//	errs() << "In module called: " << M.getName() << "!\n";

//	errs() << "AD: START\n";

	// START CallGraph traversal
	auto cg = CallGraph(M);

	//     cg.dump();

	// Initialization

	for (auto &cgn : cg)
	{
        	if (cgn.first != NULL)
        	{
			CallGraphNode *temp_node = cg.operator[](cgn.first);
			map_call_graph_node[CGN_COUNT] = temp_node;
			rev_map_call_graph_node[temp_node] = CGN_COUNT;

//			errs() << "Assigning " << CGN_COUNT << " to CallGraphNode " << cgn.first->getName() << "\n";
			
			CGN_COUNT++;
		}
	}

	unsigned int caller, callee;

//	if(cg.getExternalCallingNode() != NULL && cg.getExternalCallingNode()->getFunction() != NULL)
//		errs() << "External Calling Node " << cg.getExternalCallingNode()->getFunction()->getName() << "\n";

//	if(cg.getCallsExternalNode() != NULL)
//	errs() << "Calls External Node " << cg.getCallsExternalNode()->getFunction()->getName() << "\n";

	for (auto const &cgn : cg)
	{
		if (cgn.first != NULL)
		{
			// print the function name

			set_of_ints temp1, temp2, temp3;

			CallGraphNode *temp_node = cg.operator[](cgn.first);

			caller = rev_map_call_graph_node[temp_node];

//			errs() << "Caller: " << cgn.first->getName() << "\n";

			CallGraphNode::iterator it;
			it = cgn.second->begin();

			for(; it != cgn.second->end(); it++)
			{
				if(it->first == NULL || it->second == NULL)
					continue;

				temp_node = it->second;

				if(temp_node->getFunction() == NULL)
					continue;

				callee = rev_map_call_graph_node[temp_node];

//				errs() << "Callee: " << temp_node->getFunction()->getName() << "\n";

				temp1.insert(callee);

				temp2 = caller_rel[callee];

				temp2.insert(caller);

				caller_rel[callee] = temp2;
			}

			callee_rel[caller] = temp1;
		}
	}

//	print_call_graph();

//	checkForCallerCalleeRelForAll();

	checkForCallerCalleeInProgram(caller_name, callee_name);

	// END   CallGraph
    
    // Uncomment the below segment to access the CFG too.

    // START CFG traversal
    //// to print every instruction in every block in every function
    //for (auto &F : M) {
    //    F.dump();

    //    for (auto &B : F) {
    //        B.dump();

    //        for (auto &I : B) {
    //            I.dump();
    //        }
    //    }
    //}
    // END   CFG


//    errs() << "AD: END\n";

    return false;
}


static RegisterPass<SkeletonPass> X("skeleton", "Skeleton Pass");

/*
//Automatically enable the pass.
//http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                     legacy::PassManagerBase &PM) {
    PM.add(new SkeletonPass());
}

static RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_ModuleOptimizerEarly, registerSkeletonPass);

static RegisterStandardPasses
    RegisterMyPass0(PassManagerBuilder::EP_EnabledOnOptLevel0, registerSkeletonPass);
*/


