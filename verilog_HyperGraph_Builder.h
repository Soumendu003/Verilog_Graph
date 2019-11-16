#ifndef VERILOG_HYPERGRAPH_BUILDER_H
#define VERILOG_HYPERGRAPH_BUILDER_H
#include"verilog_Graph_Command_Processor.h"
#include"HyperGraph.h"

using namespace std;

class verilog_HyperGraph_Builder{
    private:
        verilog_Graph_Command_Processor* G_processor;
        HyperGraph* HG = 0;
        Global_Package* Graph_global_package;
        string top_module;
        vector<float>* Area_list = 0;

    public:
        verilog_HyperGraph_Builder(const char* v_file, const char* gate_file);
        ~verilog_HyperGraph_Builder(){
            delete G_processor;
            delete HG;
            delete Area_list;
        }
        void init_Builder(string top_module);
        void build_HGraph();
    
    private:
        map<uint32_t, uint32_t>* initialize_top_module();
        void expand_Module_Graph(uint16_t module_id, map<uint32_t, uint32_t>* port_to_wire);
        void build_Default_Wires();
        void dump_Node_Blocks(uint16_t top_module_id);
        void dump_Net_Lists(uint16_t top_module_id);
        map<uint32_t, uint32_t>* initialize_wire_map_module(uint16_t module_local_id, map<uint32_t, uint32_t>* port_to_wire, uint16_t instance_id, uint16_t instance_mod_id);
};




#endif