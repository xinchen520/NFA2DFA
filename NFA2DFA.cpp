//Author: Xin Chen
//Email: xinc10@illinois.edu
#include <stack>
#include <queue>
#include <iostream>
#include <set>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include <vector>
#include <sstream>
#include <ctype.h>
#define STAGE_INIT_NFA_STATE 1
#define STAGE_INIT_INPUT_SYMBOLS 2
#define STAGE_INIT_NFA_EDGES 3
#define STAGE_INIT_START_STATE 4

#define ERR_NFA_STATE_NOT_FOUND 1
#define ERR_INPUT_SYMBOL_NOT_FOUND 2
#define ERR_NFA_STATE_NOT_UNIQUE 4
#define ERR_INPUT_SYMBOL_NOT_UNIQUE 8

#define EPSILON_INPUT_SYMBOL_ID 0
using namespace std;
//TODO : need to handle error case when nfa state names are not unique.
typedef int InputSymbolID;
typedef int NFAStateID;
typedef int SubsetStateID;
struct InputSymbol
{
    InputSymbolID id;
    std::string symbolName;
};
struct NFAState
{
    NFAStateID id;
    std::string stateName;
    bool accept;
};

struct NFAEdge
{
    NFAStateID to;
    InputSymbolID input;
    bool operator< (const NFAEdge& e)const
    {
        if (to == e.to)return input < e.input;
        return to < e.to;
    }
};
struct NFAGraphNode
{
    NFAStateID stateID;
    std::set<NFAEdge> edges;
};

struct SubsetState
{
    const std::set<NFAStateID>* subset;
    SubsetStateID id;
    bool operator< (const SubsetState& s)const
    {
        return *subset < *(s.subset);
    }
};
struct SubsetEdge
{
    SubsetStateID to;
    InputSymbolID input;
};
struct SubsetGraphNode
{
    bool marked;
    SubsetStateID subsetID;
    std::vector<SubsetStateID> transition;
};
std::vector<std::string> _input_nfa_state_names;
std::vector<std::string> _input_input_symbol_names;
std::vector<std::string> _input_nfa_edge_froms;
std::vector<std::string> _input_nfa_edge_tos;
std::vector<std::string> _input_nfa_edge_symbols;
std::vector<std::string> _input_nfa_state_accpet;
std::string _input_start_nfa_state_name;

std::vector<int> _input_nfa_state_line;
std::vector<int> _input_nfa_state_accept_line;
int _input_start_nfa_state_line;
std::vector<int> _input_input_symbol_names_line;
std::vector<int> _input_nfa_edge_line;


std::vector<NFAState> _nfa_states;
std::vector<NFAGraphNode> _nfa_graph_nodes;
std::vector<InputSymbol> _input_symbols;

std::vector<std::unique_ptr<std::set<NFAStateID>>> _cache_nfa_state_epsilon;
std::vector<std::vector<std::unique_ptr<std::set<NFAStateID>>>> _cache_nfa_state_goto;

std::set<SubsetState> _subset_states_set;
std::vector<SubsetState*> _subset_states;
std::vector<SubsetGraphNode> _subset_graph_nodes;
void ReportErrorAndExit(int flag, int stage, int idx);
NFAState* LookUpNFAState(NFAStateID id);
InputSymbol* LookUpSymbol(InputSymbolID id);
SubsetState* LookUpSubsetState(const SubsetStateID id);

NFAState* FindNFAStateByName(const std::string& name);
InputSymbol* FindSymbolByName(const std::string& name);
const SubsetState* FindSubsetStateBySet(const std::set<NFAStateID>& s);
SubsetState* AddSubsetState(const std::set<NFAStateID>& s);
void AddSubsetEdge(SubsetStateID from, SubsetStateID to, InputSymbolID sym);
//do not delete the return value
std::set<NFAStateID>* EpsilonClosureNFAState(NFAStateID id);
//do not delete the return value
std::set<NFAStateID>* GotoFromNFAState(NFAStateID state_id, InputSymbolID sym_id);

//do manage the returned resource
std::set<NFAStateID>* EpsilonClosureSubset(const std::set<NFAStateID>& subset);
//do manage the returned resource
std::set<NFAStateID>* GotoFromSubsetState(SubsetStateID id, InputSymbolID sym_id);

bool IsSubsetAccept(SubsetStateID id)
{
    auto state = LookUpSubsetState(id);
    for (auto s : *(state->subset))
    {
        if (LookUpNFAState(s)->accept)return true;
    }
    return false;
}
void InitNFAStates();
void InitInputSymbols();
void InitNFAGraph();
void InitSubsetStates();
void NFA2DFA(NFAStateID start_nfa_state);
void MarkSubsetGraphNode(SubsetStateID id);
void GetInputAddEdge(const std::string& from, const std::string& to, const std::string& sym)
{
    _input_nfa_edge_froms.push_back(from);
    _input_nfa_edge_tos.push_back(to);
    _input_nfa_edge_symbols.push_back(sym);

}
void GetInput(std::istream& is)
{
    std::string line_str;
    int current_stage = 0;//0 not seen first 1 nfa state
    //3 accepting state
    int current_line = 0;
    while(getline(is,line_str))
    {
        current_line++;
        //if the line consists of spaces then skip it
        bool space_line = true;
        int len = line_str.length();
        for(int i = 0 ; i<len;++i)
        {
            if(isspace(line_str[i]))continue;
            space_line=false;break;
        }
        if(space_line)continue;
        if(line_str.find("//") == 0)
        {
            if((current_stage%2)==1)
            {
                current_stage++;continue;
            }
            else
            {
                continue;
            }
            if(current_stage == 1)
            {
                continue;
            }
        }
        else
        {
            if(current_stage%2 == 0){
                current_stage++;
                
            }
        }
        
        //std::stringstream ss(line_str);
        stringstream ss(line_str);
        if(current_stage == 1)
        {//read nfa states
            string nfa_state_name;
            ss>>nfa_state_name;
            _input_nfa_state_names.push_back(nfa_state_name);
            _input_nfa_state_line.push_back(current_line);
        }
        else if(current_stage == 3)
        {
            //read nfa accepting state
            string nfa_accept_name;
            ss>>nfa_accept_name;
            _input_nfa_state_accpet.push_back(nfa_accept_name);
            _input_nfa_state_accept_line.push_back(current_line);
        }
        else if(current_stage == 5)
        {
            //read start state
            if(_input_start_nfa_state_name == "")
            {
                ss>>_input_start_nfa_state_name;
                _input_start_nfa_state_line =current_line;
                
            }
            else
            {
                std::cerr<<"Error on line "<<current_line<<": Multiple start states.\n";
                exit(1);
            }
            
        }
        else if(current_stage == 7)
        {
            //read input symbols
            string symbol;
            ss>>symbol;
            _input_input_symbol_names.push_back(symbol);
            _input_input_symbol_names_line.push_back(current_line);
        }
        else if(current_stage == 9)
        {
            //read edges
            string from, to, sym;
            ss>>from>>to>>sym;
            _input_nfa_edge_froms.push_back(from);
            _input_nfa_edge_tos.push_back(to);
            _input_nfa_edge_symbols.push_back(sym);

            _input_nfa_edge_line.push_back(current_line);
        }
    }
}


void PrintResult(std::ostream& os);
void PrintResultDFAState(std::ostream& os);
void PrintResultDFATable(std::ostream& os);
int main()
{
    GetInput(std::cin);

    InitNFAStates();
    InitInputSymbols();

    InitNFAGraph();
    InitSubsetStates();

    NFAState* start_state = FindNFAStateByName(_input_start_nfa_state_name);

    if(!start_state)
    {
        ReportErrorAndExit(ERR_INPUT_SYMBOL_NOT_FOUND, STAGE_INIT_START_STATE, 0);
    }
    NFA2DFA(start_state->id);
    PrintResult(std::cout);
    return 0;
}

void InitNFAStates()
{
    int nfa_states_count = _input_nfa_state_names.size();//need input
    _nfa_states.resize(nfa_states_count);
    for (int i = 0; i < nfa_states_count; ++i)
    {
        NFAState s;
        s.id = i;
        s.stateName = _input_nfa_state_names[i]; //"Need Input";
        s.accept = false;
        //check for uniqueness of name
        auto end = _nfa_states.begin() + i;
        std::string& target_name = s.stateName;
        auto ret = find_if(_nfa_states.begin(), end, [&target_name](const NFAState& t) {return t.stateName == target_name; });
        if (ret != end) {
            ReportErrorAndExit(ERR_NFA_STATE_NOT_UNIQUE, STAGE_INIT_NFA_STATE, i);
        }
        //end check
        _nfa_states[i] = s;
    }
    //
    for (int i = 0; i < _input_nfa_state_accpet.size(); ++i)
    {
        std::string& s = _input_nfa_state_accpet[i];
        auto state = FindNFAStateByName(s);
        if (!state)
        {
            ReportErrorAndExit(ERR_NFA_STATE_NOT_FOUND, STAGE_INIT_NFA_STATE, i);
        }
        state->accept = true;
    }
}
void InitInputSymbols()
{
    int input_symbols_count = _input_input_symbol_names.size();//need input
    _input_symbols.resize(input_symbols_count + 1);
    //id = 0 for epsilon (empty)
    _input_symbols[0].id = 0;
    _input_symbols[0].symbolName = "##";
    for (int i = 1; i <= input_symbols_count; ++i)
    {
        InputSymbol sym;
        sym.id = i;
        sym.symbolName = _input_input_symbol_names[i - 1]; //"Need Input";

        //check for uniqueness of name
        auto end = _input_symbols.begin() + i;
        std::string& target_name = sym.symbolName;
        auto ret = find_if(_input_symbols.begin(), end, [&target_name](const InputSymbol& t) {return t.symbolName == target_name; });
        if (ret != end) {
            ReportErrorAndExit(ERR_INPUT_SYMBOL_NOT_UNIQUE, STAGE_INIT_NFA_STATE, i - 1);
        }
        //end check

        _input_symbols[i] = sym;
    }
}
void InitNFAGraph()
{
    int edges_count = _input_nfa_edge_froms.size();//need input
    int nodes_count = _nfa_states.size();
    _nfa_graph_nodes.resize(nodes_count);
    //initialize graph nodes
    for (int i = 0; i < nodes_count; ++i)
    {
        _nfa_graph_nodes[i].stateID = i;
        _nfa_graph_nodes[i].edges.clear();
    }

    //assume 
    //handle input edges
    for (int i = 0; i < edges_count; ++i)
    {
        std::string from_name = _input_nfa_edge_froms[i];//"need input from name";
        std::string to_name = _input_nfa_edge_tos[i];//"need input to name";
        std::string input_sym_name = _input_nfa_edge_symbols[i];//"need input sym";
        NFAState* from_state = FindNFAStateByName(from_name);
        NFAState* to_state = FindNFAStateByName(to_name);
        InputSymbol* sym = FindSymbolByName(input_sym_name);
        int error_flag = 0;
        if (!(from_state && to_state))
        {
            error_flag |= ERR_NFA_STATE_NOT_FOUND;
        }
        if (!sym)
        {
            error_flag |= ERR_INPUT_SYMBOL_NOT_FOUND;
        }
        if (error_flag) ReportErrorAndExit(error_flag, STAGE_INIT_NFA_EDGES, i);
        NFAEdge edge;
        edge.input = sym->id;
        edge.to = to_state->id;
        _nfa_graph_nodes[from_state->id].edges.insert(edge);
    }

    //initialize goto , epsilon cache
    int size = _nfa_states.size();
    _cache_nfa_state_epsilon.resize(size);
    _cache_nfa_state_goto.resize(size);
    for (int i = 0; i < size; ++i)
    {
        _cache_nfa_state_goto[i].resize(_input_symbols.size());
    }

}
void InitSubsetStates()
{
    AddSubsetState(std::set<NFAStateID>());//expect 0
}


void ReportErrorAndExit(int flag, int stage, int idx)
{
    std::cerr<<"Error:\n";
    if(stage == STAGE_INIT_NFA_STATE)
    {
        if(flag & ERR_NFA_STATE_NOT_UNIQUE)
        {
            std::cerr<<"On line "<<_input_nfa_state_line[idx]<<"\n";
            std::cerr<<"re-defined of state '"<<_input_nfa_state_names[idx]<<"'\n";
        }

        if(flag & ERR_NFA_STATE_NOT_FOUND)
        {
            std::cerr<<"On line "<<_input_nfa_state_accept_line[idx]<<"\n";
            std::cerr<<_input_nfa_state_accpet[idx]<<" state wasn't found in the definition.\n";
        }
    }
    else if(stage == STAGE_INIT_INPUT_SYMBOLS)
    {
        if(flag & ERR_INPUT_SYMBOL_NOT_UNIQUE)
        {
            std::cerr<<"On line "<<_input_input_symbol_names_line[idx]<<"\n";
            std::cerr<<"re-defined input symbol '"<<_input_input_symbol_names[idx]<<"'\n";
        }
    }
    else if(stage == STAGE_INIT_NFA_EDGES)
    {
        std::cerr<<"On line "<<_input_nfa_edge_line[idx]<<"\n";
        if(flag & ERR_NFA_STATE_NOT_FOUND)
        {
            std::cerr<<"NFA state not found.\n";
        }
        if(flag& ERR_INPUT_SYMBOL_NOT_FOUND)
        {
            std::cerr<<"Input symbol not found.\n";
        }
    }
    else if(stage == STAGE_INIT_START_STATE)
    {
        std::cerr<<"On line "<<_input_start_nfa_state_line<<"\n";
        std::cerr<<"Start state "<<_input_start_nfa_state_name<<" not found.\n";
    }

    exit(1);
}
NFAState* LookUpNFAState(NFAStateID id)
{
    if (id < 0 || id >= _nfa_states.size())return nullptr;
    return &_nfa_states[id];
}
InputSymbol* LookUpSymbol(InputSymbolID id)
{
    if (id < 0 || id >= _input_symbols.size())return nullptr;
    return &_input_symbols[id];
}
SubsetState* LookUpSubsetState(const SubsetStateID id)
{
    if (id < 0 || id >= _subset_states.size())return nullptr;
    return _subset_states[id];
}

NFAState* FindNFAStateByName(const std::string& name)
{
    auto it = std::find_if(_nfa_states.begin(), _nfa_states.end(), [&name](const NFAState& s) { return s.stateName == name; });
    if (it == _nfa_states.end())return nullptr;
    return &(*it);
}
InputSymbol* FindSymbolByName(const std::string& name)
{
    auto it = std::find_if(_input_symbols.begin(), _input_symbols.end(), [&name](const InputSymbol& s) { return s.symbolName == name; });
    if (it == _input_symbols.end())return nullptr;
    return &(*it);
}
const SubsetState* FindSubsetStateBySet(const std::set<NFAStateID>& s)
{
    SubsetState state;
    state.subset = &s;
    auto it = _subset_states_set.find(state);
    if (it == _subset_states_set.end())return nullptr;
    else {
        return &(*it);
    }
}
SubsetState* AddSubsetState(const std::set<NFAStateID>& s)
{
    int id = _subset_states.size();
    std::set<NFAStateID>* subset = new std::set<NFAStateID>(s);
    SubsetState state;
    state.id = id;
    state.subset = subset;
    auto ret = _subset_states_set.insert(state);
    if (!ret.second)
    {
        return nullptr;
    }
    SubsetState* s_ptr = const_cast<SubsetState*>(&(*ret.first));
    _subset_states.push_back(s_ptr);

    SubsetGraphNode node;
    node.marked = false;
    node.subsetID = id;
    node.transition.resize(_input_symbols.size());
    _subset_graph_nodes.push_back(std::move(node));
    return s_ptr;
}

std::set<NFAStateID>* EpsilonClosureNFAState(NFAStateID id)
{
    if (_cache_nfa_state_epsilon[id])
    {
        return _cache_nfa_state_epsilon[id].get();
    }

    std::set<NFAStateID>* closure = new std::set<NFAStateID>;
    std::queue<NFAStateID> last_new_added;
    closure->insert(id);
    last_new_added.push(id);
    bool changed = true;
    while (changed)
    {
        changed = false;
        int size = last_new_added.size();
        for (int i = 0; i < size; ++i)
        {
            NFAStateID  front = last_new_added.front();
            auto tos = GotoFromNFAState(front, EPSILON_INPUT_SYMBOL_ID);
            for (auto state : *tos)
            {
                auto ret = closure->insert(state);
                if (ret.second)
                {
                    last_new_added.push(state);
                    changed = true;
                }
            }
            last_new_added.pop();

        }
    }

    _cache_nfa_state_epsilon[id].reset(closure);
    return _cache_nfa_state_epsilon[id].get();
}
std::set<NFAStateID>* GotoFromNFAState(NFAStateID state_id, InputSymbolID sym_id)
{
    if (_cache_nfa_state_goto[state_id][sym_id])
    {
        return _cache_nfa_state_goto[state_id][sym_id].get();
    }
    std::set<NFAStateID>* tos = new std::set<NFAStateID>;
    for (auto& edge : _nfa_graph_nodes[state_id].edges)
    {
        if (edge.input == sym_id)
        {
            tos->insert(edge.to);
        }
    }
    _cache_nfa_state_goto[state_id][sym_id].reset(tos);
    return _cache_nfa_state_goto[state_id][sym_id].get();
}


std::set<NFAStateID>* EpsilonClosureSubset(const std::set<NFAStateID>& subset)
{
    std::set<NFAStateID>* subset_tos = new std::set<NFAStateID>;
    for (auto s : subset)
    {
        auto ep = EpsilonClosureNFAState(s);
        subset_tos->insert(ep->begin(), ep->end());
    }
    return subset_tos;
}
std::set<NFAStateID>* GotoFromSubsetState(SubsetStateID id, InputSymbolID sym_id)
{
    std::set<NFAStateID>* subset_tos = new std::set<NFAStateID>;
    auto subset_state = LookUpSubsetState(id);
    for (auto s : *(subset_state->subset))
    {
        auto tos = GotoFromNFAState(s, sym_id);
        subset_tos->insert(tos->begin(), tos->end());
    }
    return subset_tos;
}

void AddSubsetEdge(SubsetStateID from, SubsetStateID to, InputSymbolID sym)
{
    _subset_graph_nodes[from].transition[sym] = to;
}
void MarkSubsetGraphNode(SubsetStateID id)
{
    _subset_graph_nodes[id].marked = true;
}
void NFA2DFA(NFAStateID start_nfa_state)//need input start nfa state
{

    auto start_subset = AddSubsetState(*EpsilonClosureNFAState(start_nfa_state));

    std::queue<SubsetStateID> to_process;
    to_process.push(start_subset->id);
    int input_sym_count = _input_symbols.size();
    while (!to_process.empty())
    {
        SubsetStateID subset = to_process.front();
        MarkSubsetGraphNode(subset);
        to_process.pop();
        for (int i = EPSILON_INPUT_SYMBOL_ID + 1; i < input_sym_count; ++i)
        {
            auto tos = GotoFromSubsetState(subset, i);
            auto closure = EpsilonClosureSubset(*tos);
            delete tos;
            auto subset_state = FindSubsetStateBySet(*closure);
            if (subset_state)
            {
                AddSubsetEdge(subset, subset_state->id, i);
            }
            else
            {
                //construct a new node
                auto new_subset_state = AddSubsetState(*closure);
                AddSubsetEdge(subset, new_subset_state->id, i);
                to_process.push(new_subset_state->id);
            }
            delete closure;

        }
    }
}

std::ostream& operator<<(std::ostream& os, const std::set<NFAStateID>& states)
{
    os << "{";
    for (auto s : states)
    {
        auto state = LookUpNFAState(s);
        os << state->stateName << " , ";
    }
    os << "}";
    return os;
}
void PrintResult(std::ostream& os)
{
    PrintResultDFAState(os);
    PrintResultDFATable(os);
    os<<"start state, 1\n";
}
void PrintResultDFAState(std::ostream& os)
{
    for (int i = 0; i < _subset_states.size(); ++i)
    {
        auto ss = _subset_states[i];
        os << ss->id << ", " << *(ss->subset)<< "\n";
    }
}
void PrintResultDFATable(std::ostream& os)
{
    os << "state\\input ";
    for (int i = 1; i < _input_symbols.size(); ++i)
    {
        os << ", " << _input_symbols[i].symbolName;
    }
    os<<", accept";
    os << "\n";

    for (int i = 0; i < _subset_graph_nodes.size(); ++i)
    {
        auto& n = _subset_graph_nodes[i];
        os << n.subsetID;
        for (int j = 1; j < _input_symbols.size(); ++j)
        {
            os << ", " << n.transition[j];
        }
        os<<", "<< (IsSubsetAccept(n.subsetID) ? "true" :"false");
        os << "\n";
    }

}