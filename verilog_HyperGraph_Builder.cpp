#include"verilog_HyperGraph_Builder.h"

verilog_HyperGraph_Builder::verilog_HyperGraph_Builder(const char* v_file, const char* gate_file){
    this->G_processor = new verilog_Graph_Command_Processor(v_file, gate_file);
    this->HG = new HyperGraph();
    this->Area_list = new vector<float>;
}

void verilog_HyperGraph_Builder::init_Builder(string top_module){
    this->G_processor->run_Cmd_Processor();
    this->Graph_global_package = this->G_processor->copy_Global_Package();
    this->top_module = top_module;
}

void verilog_HyperGraph_Builder::build_HGraph(){
    build_Default_Wires();
    map<uint32_t, uint32_t>* port_to_wire = initialize_top_module();
    uint16_t top_module_id = this->Graph_global_package->globalSym_name_to_id->find(this->top_module)->second;
    expand_Module_Graph(top_module_id, port_to_wire);
    cout<<"No of Gates = "<<to_string(HG->get_Nodes_Size())<<endl;
    cout<<"No of Wires = "<<to_string(HG->get_Nets_Size())<<endl;
    //this->HG->build_connectivity();
    dump_Node_Blocks(top_module_id);
    dump_Net_Lists(top_module_id);
    cout<<"Graph dumped"<<endl;
}

void verilog_HyperGraph_Builder::dump_Node_Blocks(uint16_t top_module_id){
    string outfile = this->Graph_global_package->globalSym_id_to_name->at(top_module_id)+".blocks";
    string write_string;
    ofstream ofs;
    ofs.open(outfile, ofstream::out);
    for(uint32_t i = 0; i < this->HG->get_Nodes_Size(); i++){
        write_string = "bk"+to_string(i)+"\t softrectangular\t"+to_string(this->Area_list->at(i))+"\n";
        ofs.write(write_string.c_str(), strlen(write_string.c_str()));
    }
    ofs.close();
}

void verilog_HyperGraph_Builder::dump_Net_Lists(uint16_t top_module_id){
    string outfile = this->Graph_global_package->globalSym_id_to_name->at(top_module_id)+".n";
    string write_string;
    ofstream ofs;
    ofs.open(outfile, ofstream::out);
    for(uint32_t i = 0; i < this->HG->get_Nets_Size(); i++){
        write_string = "NetDegree\t:\t"+to_string(this->HG->get_degree_of_net(i))+"\n";
        ofs.write(write_string.c_str(), strlen(write_string.c_str()));

        const vector<uint32_t> nodes = this->HG->get_connected_nodes_of(i);
        for(uint32_t j = 0; j < nodes.size(); j++){
            write_string = "bk"+to_string(nodes[j])+"\n";
            ofs.write(write_string.c_str(), strlen(write_string.c_str()));
        }
    }
    ofs.close();
}

void verilog_HyperGraph_Builder::build_Default_Wires(){
    this->HG->add_net("GND");   // logic - 0. id 0
    this->HG->add_net("VPP");    // logic - 1. id 1
}

map<uint32_t, uint32_t>* verilog_HyperGraph_Builder::initialize_top_module(){
    map<uint32_t, uint32_t>* local_port_to_wire = new map<uint32_t, uint32_t>;

    uint16_t module_id = this->Graph_global_package->globalSym_name_to_id->find(this->top_module)->second;
    //cout<<"Size of top module ports = "<<to_string(this->Graph_global_package->globalSym_table->at(module_id).expandable_size)<<endl;

    local_port_to_wire->insert({0, 0});               // inserts the GND wire
    local_port_to_wire->insert({1, 1});                // inserts the VPP wire

    uint32_t resolver_id  = this->Graph_global_package->globalSym_table->at(module_id).base_resolve_id;
    uint16_t expnad_size = this->Graph_global_package->globalSym_table->at(module_id).expandable_size;
    for(uint16_t i = 0; i < expnad_size; i++){
        uint32_t wire_id, local_id = this->Graph_global_package->global_Resolver->at(resolver_id+i);
        if(local_id != 0 || local_id != 1){
            wire_id = this->HG->add_net("n_"+to_string(HG->get_Nets_Size()));
            local_port_to_wire->insert({local_id, wire_id});
        }
        
    }
    cout<<"After Initializing top module, no of net = "<<to_string(HG->get_Nets_Size())<<endl;
    return local_port_to_wire;
}

map<uint32_t, uint32_t>* verilog_HyperGraph_Builder::initialize_wire_map_module(uint16_t module_local_id, map<uint32_t, uint32_t>* port_to_wire, uint16_t instance_id, uint16_t instance_mod_id){
    uint32_t wire_id, local_id, global_port_id;
    map<uint32_t, uint32_t>* local_map = new map<uint32_t, uint32_t>;

    local_map->insert({0, 0});              // inserts "GND"
    local_map->insert({1, 1});              // inserts "VPP"

    Module_Package Module = this->Graph_global_package->Module_list->at(module_local_id);
    
    //cout<<"Instance Global name = "<<this->Graph_global_package->globalSym_id_to_name->at(instance_mod_id)<<endl;

    uint16_t expand_size = Module.symbol_table->at(instance_id).expandable_size - 1;
    uint32_t resolve_start = Module.symbol_table->at(instance_id).base_resolve_id + 1;
    //cout<<"Resolve start at = "<<to_string(resolve_start)<<endl;

    uint32_t global_start_index = this->Graph_global_package->globalSym_table->at(instance_mod_id).base_resolve_id;
    uint16_t global_expand_size = this->Graph_global_package->globalSym_table->at(instance_mod_id).expandable_size;

    if(expand_size != global_expand_size){
        cout<<"Module expand size mismatch for module = "<<this->Graph_global_package->globalSym_id_to_name->at(instance_mod_id)<<endl;
        exit(0);
    }

    for(uint32_t j = 0, k = 0; j < expand_size; j++, k++){
        //cout<<"Inside resolving"<<endl;
        local_id = Module.signal_Resolver_table->at(j+resolve_start).ref_id;
        //uint16_t symbol_id = Module.signal_Resolver_table->at(j).symbol_id;
        //cout<<"port symbol name = "<<Module.symbol_id_to_name->at(symbol_id)<<endl;
        global_port_id = this->Graph_global_package->global_Resolver->at(global_start_index + k);

        if(port_to_wire->find(local_id) != port_to_wire->end()){
            wire_id = port_to_wire->find(local_id)->second;
            local_map->insert({global_port_id, wire_id});
        }
        else{
            //cout<<"Net wire being created"<<endl;
            wire_id = this->HG->add_net("n_"+to_string(HG->get_Nets_Size()));
            port_to_wire->insert({local_id, wire_id});
            local_map->insert({local_id, wire_id});
        }
    }
    return local_map;
}

void verilog_HyperGraph_Builder::expand_Module_Graph(uint16_t module_id, map<uint32_t, uint32_t>* port_to_wire){
    uint32_t wire_id, local_id;
    uint16_t module_local_id = this->Graph_global_package->globalSym_table->at(module_id).Sym_type_id;
    Module_Package Module = this->Graph_global_package->Module_list->at(module_local_id);
    for(uint16_t i = 0; i < Module.Instance_List->size(); i++){
        uint16_t instance_id = Module.Instance_List->at(i);
        if(Module.symbol_table->at(instance_id).symbol_type == vSymbol_t::module_instance){
            uint16_t instance_mod_id = Module.signal_Resolver_table->at(Module.symbol_table->at(instance_id).base_resolve_id).ref_id;
            map<uint32_t, uint32_t>* local_port_to_wire = initialize_wire_map_module(module_local_id, port_to_wire, instance_id, instance_mod_id);
            //cout<<"calling module = "<<this->Graph_global_package->globalSym_id_to_name->at(module_id)<<endl; 
            expand_Module_Graph(instance_mod_id, local_port_to_wire);
        }
        if(Module.symbol_table->at(instance_id).symbol_type == vSymbol_t::gate_instance){
            uint32_t node_id = this->HG->add_node("bk"+to_string(this->HG->get_Nodes_Size()));
            
            // Get area information
            uint16_t instance_gate_id = Module.signal_Resolver_table->at(Module.symbol_table->at(instance_id).base_resolve_id).ref_id;
            uint16_t Instance_Symbol_id = this->Graph_global_package->globalSym_table->at(instance_gate_id).Sym_type_id;

            float area = this->Graph_global_package->Gate_Package->gate_table->at(Instance_Symbol_id).area;
            if(this->Graph_global_package->Gate_Package->gate_table->at(Instance_Symbol_id).global_id != instance_gate_id){
                cout<<"Global id mismatch for gate name = "<<this->Graph_global_package->globalSym_id_to_name->at(instance_gate_id)<<endl;
                cout<<"Gate Package name = "<<this->Graph_global_package->Gate_Package->gate_id_to_name->at(Instance_Symbol_id)<<endl;
                cout<<"Area = "<<to_string(area)<<endl;
                exit(0);
            }
            this->Area_list->push_back(area);

            uint8_t gate_expand_size = Module.symbol_table->at(instance_id).expandable_size - 1;
            uint32_t resolve_start = Module.symbol_table->at(instance_id).base_resolve_id + 1;

            for(uint32_t j = 0; j < gate_expand_size; j++){
                local_id = Module.signal_Resolver_table->at(j + resolve_start).ref_id;

                if(port_to_wire->find(local_id) == port_to_wire->end()){
                    wire_id = this->HG->add_net("n_"+to_string(HG->get_Nets_Size()));
                    port_to_wire->insert({local_id, wire_id});
                }
                else{
                    wire_id = port_to_wire->find(local_id)->second;
                }
                HG->connect_net(wire_id, node_id);
            }
        }
    }
    delete port_to_wire;
}