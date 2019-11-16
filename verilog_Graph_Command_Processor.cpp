#include"verilog_Graph_Command_Processor.h"

verilog_Graph_Command_Processor::verilog_Graph_Command_Processor(const char* v_file){
    v_Lex = new verilog_Lexer(v_file);
    tok = (verilog_token*)calloc(1, sizeof(verilog_token));
    globalSym_table = new vector<global_Symbol>;
    globalSym_id_to_name = new vector<string>;
    globalSym_name_to_id = new map<string, uint16_t>;
    global_Resolver = new vector<uint32_t>;
    global_Resolver->push_back(0);   // puts invalid element
    offsets = (uint8_t*)calloc(2,sizeof(uint8_t));
    this->Module_list = new vector<Module_Package>;   
}

verilog_Graph_Command_Processor::verilog_Graph_Command_Processor(const char* v_file, const char* gate_file){
    v_Lex = new verilog_Lexer(v_file);
    tok = (verilog_token*)calloc(1, sizeof(verilog_token));
    globalSym_table = new vector<global_Symbol>;
    globalSym_id_to_name = new vector<string>;
    globalSym_name_to_id = new map<string, uint16_t>;
    global_Resolver = new vector<uint32_t>;
    global_Resolver->push_back(0);   // puts invalid element
    offsets = (uint8_t*)calloc(2,sizeof(uint8_t));
    this->Module_list = new vector<Module_Package>;

    verilog_Gate_Parser* G_parser = new verilog_Gate_Parser(gate_file);
    G_parser->parse_Gates();
    this->Gate_Package = G_parser->copy_gate_package();
    delete G_parser;
    initialize_gates_from_file();
    
    this->gate_prefix_enable = false;
    this->gate_from_file_enable = true;  
}

verilog_Graph_Command_Processor::verilog_Graph_Command_Processor(const char* v_file, const vector<string>& gate_prefix){
    v_Lex = new verilog_Lexer(v_file);
    tok = (verilog_token*)calloc(1, sizeof(verilog_token));
    globalSym_table = new vector<global_Symbol>;
    globalSym_id_to_name = new vector<string>;
    globalSym_name_to_id = new map<string, uint16_t>;
    global_Resolver = new vector<uint32_t>;
    global_Resolver->push_back(0);   // puts invalid element
    offsets = (uint8_t*)calloc(2,sizeof(uint8_t));
    this->Module_list = new vector<Module_Package>;

    this->gate_prefix = new vector<string>;
    for(uint8_t i = 0; i < gate_prefix.size(); i++){
        this->gate_prefix->push_back(gate_prefix[i]);
    }

    this->gate_prefix_enable = true;
    this->gate_from_file_enable = false;
}

verilog_Graph_Command_Processor::verilog_Graph_Command_Processor(const char* v_file, const char* gate_file, const vector<string>& gate_prefix){
    v_Lex = new verilog_Lexer(v_file);
    tok = (verilog_token*)calloc(1, sizeof(verilog_token));
    globalSym_table = new vector<global_Symbol>;
    globalSym_id_to_name = new vector<string>;
    globalSym_name_to_id = new map<string, uint16_t>;
    global_Resolver = new vector<uint32_t>;
    global_Resolver->push_back(0);   // puts invalid element
    offsets = (uint8_t*)calloc(2,sizeof(uint8_t));
    this->Module_list = new vector<Module_Package>;

    this->gate_prefix = new vector<string>;
    for(uint8_t i = 0; i < gate_prefix.size(); i++){
        this->gate_prefix->push_back(gate_prefix[i]);
    }

    verilog_Gate_Parser* G_parser = new verilog_Gate_Parser(gate_file);
    G_parser->parse_Gates();
    this->Gate_Package = G_parser->copy_gate_package();
    delete G_parser;
    initialize_gates_from_file();

    this->gate_prefix_enable = true;
    this->gate_from_file_enable = true;
}

void verilog_Graph_Command_Processor::initialize_gates_from_file(){
    for(uint16_t i = 0; i < this->Gate_Package->gate_table->size(); i++){
        string gate_name = this->Gate_Package->gate_id_to_name->at(i);
        global_Symbol tem;
        tem.globalSym_type = vSymbol_t::gate_name;
        tem.base_resolve_id = 0;    // invalid
        tem.expandable_size = 0;
        tem.Sym_type_id = i;

        this->Gate_Package->gate_table->at(i).global_id = globalSym_table->size();

        globalSym_name_to_id->insert({gate_name, globalSym_table->size()});
        globalSym_id_to_name->push_back(gate_name);
        globalSym_table->push_back(tem);
    }
}

CommandType_t verilog_Graph_Command_Processor::get_CmdType(){
    if(tok->tok_id == Tokid_t::keyword){
        return (CommandType_t)tok->symbol_id;
    }
    if(tok->tok_id == Tokid_t::identifier){
        if(check_module_definition()){
            return CommandType_t::module_call;
        }
        if(check_gate_definition()){
            //cout<<"gate definition found"<<endl;
            return CommandType_t::gate;
        }
        cout<<"Identifier type not found in command"<<endl;
        cout<<"Command = "<<this->symbol_id_to_name->at(tok->symbol_id)<<endl;
        exit(0);
        return CommandType_t::ignore;
    }
    else{
        cout<<"Ignored Tok id = "<<to_string(tok->tok_id)<<endl;
        exit(0);
        return CommandType_t::ignore;
    }
}

bool verilog_Graph_Command_Processor::check_gate_definition(){
    string tok_name = this->symbol_id_to_name->at((tok->symbol_id));
    if(globalSym_name_to_id->find(tok_name) != globalSym_name_to_id->end()){
        uint16_t global_id = globalSym_name_to_id->find(tok_name)->second;
        if(globalSym_table->at(global_id).globalSym_type == vSymbol_t::gate_name){
            return true;
        }
    }
    if(this->gate_prefix_enable){
        for(uint8_t i = 0; i < this->gate_prefix->size(); i++){
            if(Prefix_Match(tok_name, this->gate_prefix->at(i))){
                uint16_t gate_id = this->Gate_Package->gate_table->size();

                // Inserts in the Gate Package
                Gate_t tem;
                tem.area = 0;
                tem.global_id = this->globalSym_table->size();
                this->Gate_Package->gate_name_to_id->insert({tok_name, gate_id});
                this->Gate_Package->gate_id_to_name->push_back(tok_name);
                this->Gate_Package->gate_table->push_back(tem);

                // Inserts in Global Symbol table
                global_Symbol tem1;
                tem1.globalSym_type = vSymbol_t::gate_name;
                tem1.base_resolve_id = 0;    // invalid
                tem1.expandable_size = 0;

                globalSym_name_to_id->insert({tok_name, tem.global_id});
                globalSym_id_to_name->push_back(tok_name);
                globalSym_table->push_back(tem1);

                return true;
            }
        }
    }
    return false;
}

bool verilog_Graph_Command_Processor::check_module_definition(){
    string tok_name = this->symbol_id_to_name->at((tok->symbol_id));
    if(globalSym_name_to_id->find(tok_name) != globalSym_name_to_id->end()){
        uint16_t global_id = globalSym_name_to_id->find(tok_name)->second;
        if(globalSym_table->at(global_id).globalSym_type == vSymbol_t::module_name){
            return true;
        }
    }
    return false;
}

void verilog_Graph_Command_Processor::run_Cmd_Processor(){
    //v_Lex->init_Module();
    v_Lex->get_Token(tok);
    while (tok->tok_id != Tokid_t::v_eof)
    {
        //cout<<"tok_id = "<<to_string(tok->tok_id)<<"\t symbol_id = "<<to_string(tok->symbol_id)<<endl;
        process_Cmd();
        v_Lex->get_Token(tok);
    }
}

void verilog_Graph_Command_Processor::process_module(){
    //cout<<"Inside process module "<<endl;
    // initializes new module
    v_Lex->init_Module();
    this->symbol_id_to_name = v_Lex->copy_id_to_name();
    this->symbol_name_to_id = v_Lex->copy_symbol_name_to_id();
    this->symbol_table = v_Lex->copy_symbol_table();

    this->Instance_List = new vector<uint16_t>;

    //initializes the Resolver table
    this->signal_Resolver_table = new vector<signal_Resolver>;
    signal_Resolver tem;
    tem.ref_id = 0;    // for constant logic '0'
    tem.symbol_id = 0;
    signal_Resolver_table->push_back(tem);
    tem.ref_id = 1;   // for constant logic '1'
    signal_Resolver_table->push_back(tem);


    //cout<<"\n New symbol table size = "<<to_string(this->symbol_table->size())<<endl;

    // obtained the name of the module
    v_Lex->get_Token(this->tok);                
    global_Symbol tem_symbol;
    //strncpy(tem_symbol.globalSym_name, (symbol_id_to_name->at(tok->symbol_id)).c_str(), TOK_NAME_SIZE);
    //cout<<"Module name = "<<symbol_id_to_name->at(tok->symbol_id)<<endl;
    tem_symbol.globalSym_type = vSymbol_t::module_name;

    // put it into global symbol table
    this->current_module_id = globalSym_table->size();
    globalSym_name_to_id->insert({symbol_id_to_name->at(tok->symbol_id), globalSym_table->size()});
    globalSym_id_to_name->push_back(symbol_id_to_name->at(tok->symbol_id));
    globalSym_table->push_back(tem_symbol);

    // gets all the ports information of that module
    initial_port_list.clear();
    v_Lex->get_Token(this->tok);
    while (tok->tok_id != Tokid_t::semi_colon)
    {
        if(tok->tok_id == Tokid_t::identifier){
            initial_port_list.push_back(tok->symbol_id);
        }
        v_Lex->get_Token(this->tok);
    }

    /*for(uint8_t i = 0; i < initial_port_list.size(); i++){
        cout<<"For module = "<<tem_symbol.globalSym_name<<"\tPort = "<<symbol_id_to_name->at(initial_port_list[i])<<endl;
    }*/
}

void verilog_Graph_Command_Processor::process_input(){
    uint8_t max_size = 1, min_size = 0;
    v_Lex->get_Token(this->tok);
    while (tok->tok_id != Tokid_t::semi_colon)
    {
        if(tok->tok_id == Tokid_t::lb){
            v_Lex->get_Token(this->tok);           // gets max_size
            if(tok->tok_id != Tokid_t::integer){
                cout<<"Max Size is not an integer"<<endl;
                exit(0);
            }
            max_size = tok->symbol_id+1;

            v_Lex->get_Token(this->tok);          // gets ':'
            if(tok->tok_id != Tokid_t::colon){
                cout<<"Colon not found in size "<<endl;
                exit(0);
            }

            v_Lex->get_Token(this->tok);           // gets max_size
            if(tok->tok_id != Tokid_t::integer){
                cout<<"Min Size is not an integer"<<endl;
                exit(0);
            }
            min_size = tok->symbol_id;

            if(min_size != 0){
                cout<<"Min size is not 0"<<endl;
                exit(0);
            }

            v_Lex->get_Token(this->tok);
            if(tok->tok_id != Tokid_t::rb){
                cout<<"Right backet not found in size"<<endl;
                exit(0);
            }
        }
        else{
            if(tok->tok_id == Tokid_t::identifier){
                symbol_table->at(tok->symbol_id).expandable_size = max_size - min_size;
                if(symbol_table->at(tok->symbol_id).symbol_type != vSymbol_t::unknown){
                    cout<<"Can't assign as \"input\". while previously assigned"<<endl;
                    exit(0);
                }
                symbol_table->at(tok->symbol_id).symbol_type = vSymbol_t::input_instance;
                symbol_table->at(tok->symbol_id).base_resolve_id = this->signal_Resolver_table->size();

                signal_Resolver tem;
                tem.symbol_id = tok->symbol_id;
                for(uint16_t i = 0; i < symbol_table->at(tok->symbol_id).expandable_size; i++){
                    tem.ref_id = this->signal_Resolver_table->size();
                    this->signal_Resolver_table->push_back(tem);
                }
                //cout<<"Input Port name = "<<symbol_id_to_name->at(tok->symbol_id)<<"\t symbol id = "<<to_string(tok->symbol_id)<<"\t expand size = "<<to_string( symbol_table->at(tok->symbol_id).expandable_size)<<endl;
            }
        }
        v_Lex->get_Token(this->tok);
    }
}

void verilog_Graph_Command_Processor::process_output(){
    uint8_t max_size = 1, min_size = 0;
    v_Lex->get_Token(this->tok);
    while (tok->tok_id != Tokid_t::semi_colon)
    {
        if(tok->tok_id == Tokid_t::lb){
            v_Lex->get_Token(this->tok);           // gets max_size
            if(tok->tok_id != Tokid_t::integer){
                cout<<"Max Size is not an integer"<<endl;
                exit(0);
            }
            max_size = tok->symbol_id+1;

            v_Lex->get_Token(this->tok);          // gets ':'
            if(tok->tok_id != Tokid_t::colon){
                cout<<"Colon not found in size "<<endl;
                exit(0);
            }

            v_Lex->get_Token(this->tok);           // gets max_size
            if(tok->tok_id != Tokid_t::integer){
                cout<<"Min Size is not an integer"<<endl;
                exit(0);
            }
            min_size = tok->symbol_id;

            if(min_size != 0){
                cout<<"Min size is not 0"<<endl;
                exit(0);
            }

            v_Lex->get_Token(this->tok);
            if(tok->tok_id != Tokid_t::rb){
                cout<<"Right backet not found in size"<<endl;
                exit(0);
            }
        }
        else{
            if(tok->tok_id == Tokid_t::identifier){
                symbol_table->at(tok->symbol_id).expandable_size = max_size - min_size;
                if(symbol_table->at(tok->symbol_id).symbol_type != vSymbol_t::unknown){
                    cout<<"Can't assign as \"output\". while previously assigned"<<endl;
                    exit(0);
                }
                symbol_table->at(tok->symbol_id).symbol_type = vSymbol_t::output_instance;
                symbol_table->at(tok->symbol_id).base_resolve_id = this->signal_Resolver_table->size();

                signal_Resolver tem;
                tem.symbol_id = tok->symbol_id;
                for(uint16_t i = 0; i < symbol_table->at(tok->symbol_id).expandable_size; i++){
                    tem.ref_id = this->signal_Resolver_table->size();
                    this->signal_Resolver_table->push_back(tem);
                }
                //cout<<"Output Port name = "<<symbol_id_to_name->at(tok->symbol_id)<<"\t symbol id = "<<to_string(tok->symbol_id)<<"\t expand size = "<<to_string( symbol_table->at(tok->symbol_id).expandable_size)<<endl;
            }
        }
        v_Lex->get_Token(this->tok);
    }
}

void verilog_Graph_Command_Processor::process_wire(){
    //cout<<"Parsing Wire"<<endl;
    uint8_t max_size = 1, min_size = 0;
    v_Lex->get_Token(this->tok);
    while (tok->tok_id != Tokid_t::semi_colon)
    {
        if(tok->tok_id == Tokid_t::lb){
            v_Lex->get_Token(this->tok);           // gets max_size
            if(tok->tok_id != Tokid_t::integer){
                cout<<"Max Size is not an integer"<<endl;
                exit(0);
            }
            max_size = tok->symbol_id+1;
            //cout<<"Max size = "<<to_string(max_size)<<endl;

            v_Lex->get_Token(this->tok);          // gets ':'
            if(tok->tok_id != Tokid_t::colon){
                cout<<"Colon not found in size . Module name = "<<globalSym_id_to_name->at(globalSym_id_to_name->size()-1)<<endl;
                cout<<"Max size found = "<<to_string(max_size)<<endl;
                cout<<"tok id = "<<to_string(tok->tok_id)<<endl;
                cout<<"Expected id "<<to_string(Tokid_t::colon)<<endl;
                exit(0);
            }

            v_Lex->get_Token(this->tok);           // gets max_size
            if(tok->tok_id != Tokid_t::integer){
                cout<<"Min Size is not an integer"<<endl;
                exit(0);
            }
            min_size = tok->symbol_id;
            //cout<<"Min size = "<<to_string(min_size)<<endl;

            if(min_size != 0){
                cout<<"Min size is not 0"<<endl;
                exit(0);
            }

            v_Lex->get_Token(this->tok);
            if(tok->tok_id != Tokid_t::rb){
                cout<<"Right backet not found in size"<<endl;
                exit(0);
            }
        }
        else{
            if(tok->tok_id == Tokid_t::identifier){
                if(symbol_table->at(tok->symbol_id).symbol_type == vSymbol_t::unknown){
                    symbol_table->at(tok->symbol_id).expandable_size = max_size - min_size;
                    symbol_table->at(tok->symbol_id).symbol_type = vSymbol_t::wire_instance;
                    symbol_table->at(tok->symbol_id).base_resolve_id = this->signal_Resolver_table->size();

                    signal_Resolver tem;
                    tem.symbol_id = tok->symbol_id;
                    for(uint16_t i = 0; i < symbol_table->at(tok->symbol_id).expandable_size; i++){
                        tem.ref_id = this->signal_Resolver_table->size();
                        this->signal_Resolver_table->push_back(tem);
                    }
                    //cout<<"Wire Port name = "<<symbol_id_to_name->at(tok->symbol_id)<<"\t symbol id = "<<to_string(tok->symbol_id)<<"\t expand size = "<<to_string(symbol_table->at(tok->symbol_id).expandable_size)<<endl;
                }
            }
        }
        v_Lex->get_Token(this->tok);
    }
}

void verilog_Graph_Command_Processor::process_assign(){
    uint16_t r_symbol_id, l_symbol_id;
    uint32_t l_resolve_id, r_resolve_id;
    uint8_t l_offset = -1, r_offset = -1;

    this->v_Lex->get_Token(this->tok);  // Gets lvalue
    l_symbol_id = tok->symbol_id;
    l_resolve_id = this->symbol_table->at(l_symbol_id).base_resolve_id;

    this->v_Lex->get_Token(this->tok);  // Gets '=' or '['

    if(tok->tok_id == Tokid_t::lb){
        this->v_Lex->get_Token(this->tok);  // gets offset
        if(tok->tok_id != Tokid_t::integer){
            cout<<"Offset in assign must be integer"<<endl;
            exit(0);
        }
        uint8_t l_offset = tok->symbol_id;

        this->v_Lex->get_Token(this->tok);  // gets ']'
        if(tok->tok_id != Tokid_t::rb){
            cout<<"Closing ']' not found in l_val of assign"<<endl;
            exit(0);
        }

        this->v_Lex->get_Token(this->tok);  // gets '='
    }

    if( tok->tok_id != Tokid_t::equal){
        cout<<" = not found in assign"<<endl;
        exit(0);
    }

    this->v_Lex->get_Token(this->tok);   // Gets rvalue
    r_symbol_id = get_port_symbol_id();

    if(r_symbol_id == 0){              // binary constant
        const vector<uint8_t> values = get_value_list();
        if(l_offset == uint8_t(-1)){              //i.e offset == -1
            for(uint8_t i = 0 ; i < values.size(); i++){
                this->signal_Resolver_table->at(l_resolve_id + i).ref_id = values[i];
            }
        }
        else{

            this->signal_Resolver_table->at(l_resolve_id + l_offset).ref_id  = values[0];
        }
    }

    this->v_Lex->get_Token(this->tok);  // Gets '[' or ';'

    if(tok->tok_id == Tokid_t::lb){
        this->v_Lex->get_Token(this->tok);  // gets offset
        if(tok->tok_id != Tokid_t::integer){
            cout<<"Offset in assign must be integer"<<endl;
            exit(0);
        }
        uint8_t r_offset = tok->symbol_id;

        this->v_Lex->get_Token(this->tok);  // gets ']'
        if(tok->tok_id != Tokid_t::rb){
            cout<<"Closing ']' not found in r_val of assign"<<endl;
            exit(0);
        }

        this->v_Lex->get_Token(this->tok);  // gets ';'
    }

    if( tok->tok_id != Tokid_t::semi_colon){
        cout<<" ; not found in assign"<<endl;
        exit(0);
    }

    if(r_symbol_id != 0){
        r_resolve_id = this->symbol_table->at(r_symbol_id).base_resolve_id;
        if(l_offset == uint8_t(-1) && r_offset == uint8_t(-1)){
            for(uint8_t i = 0; i < this->symbol_table->at(l_resolve_id).expandable_size; i++){
                signal_Resolver_table->at(l_resolve_id + i).ref_id = signal_Resolver_table->at(r_resolve_id + i).ref_id;
            }
        }
        else if(l_offset == uint8_t(-1)){
            signal_Resolver_table->at(l_resolve_id).ref_id = signal_Resolver_table->at(r_resolve_id + r_offset).ref_id;
        }
        else if(r_offset == uint8_t(-1)){
            signal_Resolver_table->at(l_resolve_id + l_offset).ref_id = signal_Resolver_table->at(r_resolve_id).ref_id;
        }
        else{
            signal_Resolver_table->at(l_resolve_id + l_offset).ref_id = signal_Resolver_table->at(r_resolve_id + r_offset).ref_id;
        }
    }

}


uint16_t verilog_Graph_Command_Processor::process_module_ports(uint16_t instance_symbol_id, bool port_name_parse){
    this->v_Lex->get_Token(this->tok);

    if(tok->tok_id == Tokid_t::lb){        // case e.g SDFFQX1 \n6m_reg[0][0] (.CK (clock_c), .D (\n6m[0] [0]), .SI (i2[0]), .SE (n_163), .Q (\n6m[0] [0]));
        string symbol_name = this->symbol_id_to_name->at(instance_symbol_id);
        symbol_name += "[";
        this->v_Lex->get_Token(this->tok);
        symbol_name += to_string(tok->symbol_id);    // append the int value
        this->v_Lex->get_Token(this->tok);          // gets ']'
        if(tok->tok_id != Tokid_t::rb){
            cout<<"']' not found for complext instance name."<<endl;
            exit(0);
        }
        symbol_name += "]";

        symbol_name_to_id->insert({symbol_name, symbol_table->size()});
        symbol_id_to_name->push_back(symbol_name);
        verilog_symbol tem_symbol;
        tem_symbol.symbol_type = vSymbol_t::unknown;
        tem_symbol.expandable_size = 0;
        tem_symbol.base_resolve_id = 0;
        instance_symbol_id = symbol_table->size();
        symbol_table->push_back(tem_symbol);
    }
    else if(tok->tok_id != Tokid_t::lp){
        cout<<"Expected '(' . But not found in module instance."<<endl;
        cout<<"Instance name = "<<this->symbol_id_to_name->at(instance_symbol_id)<<endl;
        exit(0);
    }
    
    uint16_t port_id = 0;
    this->v_Lex->get_Token(this->tok);
    while(tok->tok_id != Tokid_t::semi_colon){
        if(tok->tok_id == Tokid_t::identifier){
            port_id = resolve_port_variable(instance_symbol_id, port_id);
            if(!port_name_parse){
                port_id = 0;
            }
        }
        this->v_Lex->get_Token(this->tok);
    }
    return instance_symbol_id;
}

uint16_t verilog_Graph_Command_Processor::get_port_symbol_id(){
    string variable_name = this->symbol_id_to_name->at(this->tok->symbol_id);
    //cout<<"Variable name == "<<variable_name<<endl;
    if(variable_name.find(string("'b")) != string::npos){
        return 0;
    }
    else if(variable_name.at(0) == '.'){
        return -1;
    }
    else{
        return tok->symbol_id;
    }
    
}

const vector<uint8_t>& verilog_Graph_Command_Processor::get_value_list(){
    string variable_name = this->symbol_id_to_name->at(this->tok->symbol_id);
    size_t index = variable_name.find(string("'b"));
    static vector<uint8_t> values;
    values.clear();

    for(size_t i = index+2; i < variable_name.size(); i++){
        if(variable_name[i] == '0'){
            values.push_back(0);
        }
        else if(variable_name[i] == '1'){
            values.push_back(1);
        }
        else{
            cout<<"Undefined constant value found"<<endl;
            exit(0);
        }
    }
    return values;
}

void verilog_Graph_Command_Processor::get_port_offsets(uint16_t var_symbol_id){
    this->v_Lex->get_Token(this->tok);
    if(tok->tok_id == Tokid_t::comma || tok->tok_id == Tokid_t::rc || tok->tok_id == Tokid_t::rp){
        this->offsets[0] = 0;
        this->offsets[1] = this->symbol_table->at(var_symbol_id).expandable_size -1 ;
        return;
    }
    else if(tok->tok_id == Tokid_t::lb){
        uint8_t max_size, min_size;

        v_Lex->get_Token(this->tok);           // gets max_size
        if(tok->tok_id != Tokid_t::integer){
            cout<<"Max Size is not an integer"<<endl;
            exit(0);
        }
        max_size = tok->symbol_id;

        v_Lex->get_Token(this->tok);     //  gets ':' or ']'
        if(tok->tok_id == Tokid_t::colon){
            v_Lex->get_Token(this->tok);           // gets min_size
            if(tok->tok_id != Tokid_t::integer){
                cout<<"Min Size is not an integer"<<endl;
                exit(0);
            }
            min_size = tok->symbol_id;

            this->offsets[0] = min_size;
            this->offsets[1] = max_size;

            v_Lex->get_Token(this->tok);           // gets ']'
        }
        else{
            this->offsets[0] = this->offsets[1] = max_size;
        }

        if(tok->tok_id != Tokid_t::rb){
            cout<<"Closing ']' not found"<<endl;
            cout<<"Max size = "<<to_string(this->offsets[1])<<endl;
            cout<<"Min size = "<<to_string(this->offsets[0])<<endl;
            exit(0);
        }
    }
    else{
        cout<<"Invalid token found. while getting the port offsets"<<endl;
        cout<<"Token id = "<<to_string(tok->tok_id)<<endl;
        cout<<"Obatined token = "<<this->symbol_id_to_name->at(tok->symbol_id)<<endl;
        exit(0);
    }
}

uint16_t verilog_Graph_Command_Processor::get_global_port_id(){
    uint16_t port_id = 0;
    string port_name = this->symbol_id_to_name->at(tok->symbol_id);
    if(this->globalSym_name_to_id->find(port_name) != this->globalSym_name_to_id->end()){
        port_id = this->globalSym_name_to_id->find(port_name)->second;
        if(this->globalSym_table->at(port_id).globalSym_type != vSymbol_t::port_name){
            cout<<"Port name-id mismatch"<<endl;
            exit(0);
        }
        return port_id;
    }
    else{
        global_Symbol tem;
        tem.base_resolve_id = 0;
        tem.expandable_size = 0;
        tem.globalSym_type = vSymbol_t::port_name;

        uint16_t global_id = this->globalSym_table->size();
        this->globalSym_name_to_id->insert({port_name, global_id});
        this->globalSym_id_to_name->push_back(port_name);
        this->globalSym_table->push_back(tem);
        return global_id;
    }
    
}

uint16_t verilog_Graph_Command_Processor::resolve_port_variable(uint16_t instance_symbol_id, uint16_t port_id){
    signal_Resolver tem;
    tem.symbol_id = instance_symbol_id;
    
    uint16_t symbol_id = get_port_symbol_id();
    //cout<<"symbol id = "<<to_string(symbol_id)<<endl;
    if(symbol_id == uint16_t(-1)){
        port_id = get_global_port_id();
        return port_id;
    }
    else if(symbol_id == 0){       // Constant Binary
        const vector<uint8_t> value_list = get_value_list();
        for(uint8_t i = 0; i < value_list.size(); i++){
            tem.ref_id = value_list[i];
            this->signal_Resolver_table->push_back(tem);
            if(port_id != 0){
                this->global_Resolver->push_back(port_id);
                //cout<<"Port name = "<<this->globalSym_id_to_name->at(port_id)<<endl;
            }
            
        }
    }
    else{
        get_port_offsets(symbol_id);
        /*cout<<"Port name = "<<this->symbol_id_to_name->at(symbol_id)<<endl;
        cout<<"lowest offset = "<<to_string(this->offsets[0])<<endl;
        cout<<"highest offset = "<<to_string(this->offsets[1])<<endl;*/
        for(uint8_t i = this->offsets[0]; i <= this->offsets[1]; i++){
            if(this->symbol_table->at(symbol_id).base_resolve_id +i >= signal_Resolver_table->size()){
                cout<<"Signal Resolver table size out of bound "<<endl;
            }
            tem.ref_id = this->signal_Resolver_table->at(this->symbol_table->at(symbol_id).base_resolve_id +i).ref_id;
            this->signal_Resolver_table->push_back(tem);
            if(port_id != 0){
                this->global_Resolver->push_back(port_id);
                //cout<<"Port name = "<<this->globalSym_id_to_name->at(port_id)<<endl;
            }
        }
    }
    return port_id;
}

void verilog_Graph_Command_Processor::process_module_call(){
    uint16_t global_id = globalSym_name_to_id->find(symbol_id_to_name->at(tok->symbol_id))->second;
    //cout<<"Module instance name = "<<globalSym_id_to_name->at(global_id)<<endl;
    //cout<<"Global id = "<<to_string(global_id)<<endl;

    // gets the name of the module instance
    v_Lex->get_Token(this->tok);

    // Resolver table entry 
    signal_Resolver tem;
    tem.ref_id = global_id;
    tem.symbol_id = tok->symbol_id;
    symbol_table->at(tok->symbol_id).base_resolve_id = this->signal_Resolver_table->size();
    symbol_table->at(tem.symbol_id).symbol_type = vSymbol_t::module_instance;
    this->signal_Resolver_table->push_back(tem);

    // processes ports of module instance
    uint32_t start = this->signal_Resolver_table->size();
    process_module_ports(tem.symbol_id, false);
    uint32_t end = this->signal_Resolver_table->size();

    this->Instance_List->push_back(tem.symbol_id);

    //cout<<"Size = "<<to_string(end-start)<<endl;
    this->globalSym_table->at(global_id).expandable_size = (end - start);
    /*if((end - start) != this->globalSym_table->at(global_id).expandable_size){
        cout<<"No of ports mismatch ."<<endl;
        exit(0);
    }*/
    symbol_table->at(tem.symbol_id).expandable_size = this->globalSym_table->at(global_id).expandable_size + 1;
}

void verilog_Graph_Command_Processor::process_gate(){
    uint16_t ret_symbol_id, global_id = globalSym_name_to_id->find(symbol_id_to_name->at(tok->symbol_id))->second;
    //cout<<"Gate instance name = "<<this->globalSym_id_to_name->at(global_id)<<endl;

    // gets the name of the gate instance
    v_Lex->get_Token(this->tok);

    // Resolver table entry
    signal_Resolver tem;
    tem.ref_id = global_id;
    tem.symbol_id = tok->symbol_id;
    uint32_t pre_base_id = symbol_table->at(tem.symbol_id).base_resolve_id;
    symbol_table->at(tem.symbol_id).base_resolve_id = this->signal_Resolver_table->size();
    vSymbol_t pre_type = symbol_table->at(tem.symbol_id).symbol_type;
    symbol_table->at(tem.symbol_id).symbol_type = vSymbol_t::gate_instance;
    this->signal_Resolver_table->push_back(tem);

    // processes ports of gate instance
    uint32_t start = this->signal_Resolver_table->size();
    if(this->globalSym_table->at(global_id).base_resolve_id == 0){
        this->globalSym_table->at(global_id).base_resolve_id = this->global_Resolver->size();
        //cout<<"Gate instance name = "<<this->globalSym_id_to_name->at(global_id)<<endl;
        //cout<<"Base resolver id = "<<to_string(this->globalSym_table->at(global_id).base_resolve_id)<<endl;
        ret_symbol_id = process_module_ports(tem.symbol_id, true);
        this->globalSym_table->at(global_id).expandable_size = this->global_Resolver->size() - this->globalSym_table->at(global_id).base_resolve_id;
        //cout<<"Expandable size = "<<to_string(this->globalSym_table->at(global_id).expandable_size)<<endl;
    }
    else{
        ret_symbol_id = process_module_ports(tem.symbol_id, false);
    }
    uint32_t end = this->signal_Resolver_table->size();

    if(tem.symbol_id != ret_symbol_id){
        this->signal_Resolver_table->at(start - 1).symbol_id = ret_symbol_id;
        symbol_table->at(tem.symbol_id).base_resolve_id = pre_base_id;
        symbol_table->at(tem.symbol_id).symbol_type = pre_type;
        symbol_table->at(ret_symbol_id).base_resolve_id = start - 1;
        symbol_table->at(ret_symbol_id).symbol_type = vSymbol_t::gate_instance;

        /*cout<<"Previous instance name = "<<symbol_id_to_name->at(tem.symbol_id)<<endl;
        cout<<" Mismatched instance name = "<<symbol_id_to_name->at(ret_symbol_id)<<endl;
        cout<<"sent symbol_id = "<<to_string(tem.symbol_id)<<"\t received symbol_id = "<<to_string(ret_symbol_id)<<endl;
        */
    }

    this->Instance_List->push_back(ret_symbol_id);

    //cout<<"Size = "<<to_string(end-start)<<endl;
    this->globalSym_table->at(global_id).expandable_size = (end - start);
    /*if((end - start) != this->globalSym_table->at(global_id).expandable_size){
        cout<<"No of ports mismatch ."<<endl;
        exit(0);
    }*/

    symbol_table->at(tem.symbol_id).expandable_size = this->globalSym_table->at(global_id).expandable_size + 1;
}

void verilog_Graph_Command_Processor::process_endmodule(){
    if(this->globalSym_table->at(current_module_id).globalSym_type != vSymbol_t::module_name){
        cout<<"Current module id is not of a Module"<<endl;
        exit(0);
    }
    this->globalSym_table->at(current_module_id).base_resolve_id = this->global_Resolver->size();
    for(uint8_t i = 0; i < this->initial_port_list.size(); i++){
        for(uint8_t j = 0; j < this->symbol_table->at(this->initial_port_list[i]).expandable_size; j++){
            this->global_Resolver->push_back(this->symbol_table->at(this->initial_port_list[i]).base_resolve_id + j);
        }
    }
    this->globalSym_table->at(current_module_id).expandable_size = this->global_Resolver->size() - this->globalSym_table->at(current_module_id).base_resolve_id;
    this->globalSym_table->at(current_module_id).Sym_type_id = this->Module_list->size();

    // Add all the resources in Module Packet
    Module_Package tem;
    tem.global_id = current_module_id;
    tem.Instance_List = this->Instance_List;
    tem.signal_Resolver_table = this->signal_Resolver_table;
    tem.symbol_id_to_name = this->symbol_id_to_name;
    tem.symbol_name_to_id = this->symbol_name_to_id;
    tem.symbol_table = this->symbol_table;

    this->Module_list->push_back(tem);
}

Global_Package* verilog_Graph_Command_Processor::copy_Global_Package(){
    Global_Package* tem = (Global_Package*)calloc(1, sizeof(Global_Package));
    tem->Gate_Package = this->Gate_Package;
    tem->global_Resolver = this->global_Resolver;
    tem->globalSym_id_to_name = this->globalSym_id_to_name;
    tem->globalSym_name_to_id = this->globalSym_name_to_id;
    tem->globalSym_table = this->globalSym_table;
    tem->Module_list = this->Module_list;

    return tem;
}

void verilog_Graph_Command_Processor::delete_Global_packet(){
    delete this->global_Resolver;
    delete this->globalSym_id_to_name;
    delete this->globalSym_name_to_id;
    delete globalSym_table;

    delete this->Gate_Package->gate_id_to_name;
    delete this->Gate_Package->gate_name_to_id;
    delete this->Gate_Package->gate_table;
    delete this->Gate_Package;

    for(uint16_t i = 0; i < Module_list->size(); i++){
        delete this->Module_list->at(i).symbol_name_to_id;
        delete this->Module_list->at(i).symbol_id_to_name;
        delete this->Module_list->at(i).symbol_table;
        delete this->Module_list->at(i).signal_Resolver_table;
        delete this->Module_list->at(i).Instance_List;
    }
    delete this->Module_list;
    cout<<"All Parse Tables deleted"<<endl;
}