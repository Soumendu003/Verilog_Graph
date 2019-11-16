#ifndef VERILOG_GRAPH_COMMAND_PROCESSOR_H
#define VERILOG_GRAPH_COMMAND_PROCESSOR_H

#include"verilog_Command_Processor.h"
#include"verilog_Lexer.h"
#include"verilog_Gate_Parser.h"

struct global_Symbol{
    vSymbol_t globalSym_type;
    uint16_t expandable_size;
    uint32_t base_resolve_id;
    uint16_t Sym_type_id;
};

struct signal_Resolver{
    uint32_t ref_id;
    uint16_t symbol_id;
};

struct Module_Package{
    map<string, uint16_t>* symbol_name_to_id;
    vector<string>* symbol_id_to_name;
    vector<verilog_symbol>* symbol_table;
    vector<signal_Resolver>* signal_Resolver_table;
    vector<uint16_t>* Instance_List;
    uint16_t global_id;
};

struct Global_Package{
    map<string, uint16_t>* globalSym_name_to_id;
    vector<string>* globalSym_id_to_name;
    vector<global_Symbol>* globalSym_table;
    vector<uint32_t>* global_Resolver;
    gate_package* Gate_Package;
    vector<Module_Package>* Module_list;
};


class verilog_Graph_Command_Processor:public verilog_Command_Processor{
    private:
        verilog_Lexer* v_Lex = 0;
        verilog_token* tok = 0;
        map<string, uint16_t>* symbol_name_to_id = 0;
        vector<string>* symbol_id_to_name = 0;
        vector<verilog_symbol>* symbol_table = 0;
        map<string, uint16_t>* globalSym_name_to_id = 0;
        vector<string>* globalSym_id_to_name = 0;
        vector<global_Symbol>* globalSym_table = 0;
        vector<uint16_t> initial_port_list;
        vector<uint32_t>* global_Resolver = 0;
        vector<signal_Resolver>* signal_Resolver_table = 0;
        uint8_t* offsets = 0;
        vector<Module_Package>* Module_list = 0;
    
    private:
        bool gate_prefix_enable = false;
        bool gate_from_file_enable = false;
        vector<string>* gate_prefix = 0;
        gate_package* Gate_Package = 0;
        vector<uint16_t>* Instance_List = 0;
        uint16_t current_module_id = -1;
        Global_Package* global_packet = 0;

    public:
        explicit verilog_Graph_Command_Processor(){};
        explicit verilog_Graph_Command_Processor(const char* v_file);
        explicit verilog_Graph_Command_Processor(const char* v_file, const char* gate_file);
        explicit verilog_Graph_Command_Processor(const char* v_file, const vector<string>& gate_prefix);
        explicit verilog_Graph_Command_Processor(const char* v_file, const char* gate_file, const vector<string>& gate_prefix);
        ~verilog_Graph_Command_Processor(){
            if(tok != 0){
                free(tok);
            }
            if(offsets != 0){
                free(offsets);
            }
            delete v_Lex;
            delete gate_prefix;
            delete_Global_packet();
            delete global_packet;
        }
        void run_Cmd_Processor();
        Global_Package* copy_Global_Package();

    private:
        CommandType_t get_CmdType();
        void process_module();
        void process_input();
        void process_output();
        void process_wire();
        void process_gate();
        void process_endmodule();
        void process_module_call();
        void process_assign();

    private:
        bool check_module_definition();
        bool check_gate_definition();
        uint16_t process_module_ports(uint16_t instance_symbol_id, bool port_name_parse);
        uint16_t resolve_port_variable(uint16_t instance_symbol_id, uint16_t port_id);
        uint16_t get_port_symbol_id();
        const vector<uint8_t>& get_value_list();
        void get_port_offsets(uint16_t var_symbol_id);
        void initialize_gates_from_file();
        uint16_t get_global_port_id();
        void delete_Global_packet();
};

#endif
