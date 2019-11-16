#ifndef VERILOG_GATE_PARSER_H
#define VERILOG_GATE_PARSER_H
#include"ut_Tokenizer.h"
#include"Header.h"

struct Gate_t{
    float area;
    uint16_t global_id;
};

struct gate_package{
    vector<Gate_t>* gate_table;
    map<string, uint16_t>* gate_name_to_id;
    vector<string>* gate_id_to_name;
};

class verilog_Gate_Parser{

    private:
        ut_Tokenizer* Tok = 0;
        string Special_Char_Set = "(){}[]:,;";
        string  Delimeters = "\n\t\r";
        vector<Gate_t>* gate_table = 0;
        map<string, uint16_t>* gate_name_to_id = 0;
        vector<string>* gate_id_to_name = 0;

    public:
        explicit verilog_Gate_Parser(const char* file){
            Tok = new ut_Tokenizer(file, Delimeters.c_str(), Special_Char_Set.c_str());
            gate_table = new vector<Gate_t>;
            gate_id_to_name = new vector<string>;
            gate_name_to_id = new map<string, uint16_t>;
        }
        ~verilog_Gate_Parser(){
            delete Tok;
        }
        void parse_Gates(){
            char* tok = Tok->get_token();
            while(tok != 0){
                string gate_name = string(tok);
                tok = Tok->get_token();
                uint32_t no_of_instance = stoi(string(tok)); 
                tok = Tok->get_token();   // area
                float area = stof(string(tok))/no_of_instance;
                tok = Tok->get_token();   // library type

                Gate_t tem;
                tem.area = area;
                tem.global_id = 0;
                gate_name_to_id->insert({gate_name, gate_table->size()});
                gate_id_to_name->push_back(gate_name);
                gate_table->push_back(tem);

                tok = Tok->get_token();  //  gate name 
            }
        }
        gate_package* copy_gate_package(){
            gate_package* ret = (gate_package*)calloc(1, sizeof(gate_package));

            ret->gate_id_to_name = this->gate_id_to_name;
            ret->gate_name_to_id = this->gate_name_to_id;
            ret->gate_table = this->gate_table;

            return ret;
        }
};

#endif