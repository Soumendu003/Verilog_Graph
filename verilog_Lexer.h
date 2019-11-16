#ifndef VERILOG_LEXER_H
#define VERILOG_LEXER_H
#include"ut_Tokenizer.h"
#include"Header.h"
#include<cctype>
#include"verilog_Command_Processor.h"
#include"string.h"
#include<map>

using namespace std;

#define TOK_NAME_SIZE 64

enum Tokid_t{
    lp,                 //  '('
    rp,                 //  ')'
    lc,                 //  '{'
    rc,                 //  '}'
    lb,                 //  '['           
    rb,                 //  ']'
    colon,              //  ':'
    semi_colon,         //  ';'
    comma,              //  ','
    identifier,
    integer,     
    keyword,
    equal,              //  '='
    v_eof
};

enum vSymbol_t{
    module_name,
    module_instance,
    gate_name,
    gate_instance,
    port_name,
    input_instance,
    output_instance,
    wire_instance,
    unknown
};

struct verilog_token{
    Tokid_t tok_id;
    uint16_t symbol_id;
};

struct verilog_symbol{
    vSymbol_t symbol_type;
    uint16_t expandable_size;
    uint32_t base_resolve_id;             // starting id at Resolver table
};

const map<string, CommandType_t> V_keywords = {
    {"module", CommandType_t::module},
    {"input", CommandType_t::input},
    {"output", CommandType_t::output},
    {"wire", CommandType_t::wire},
    {"endmodule", CommandType_t::endmodule},
    {"assign", CommandType_t::assign}
};


class verilog_Lexer{
    private:
        ut_Tokenizer* Tok = 0;
        Tokid_t tok_id;
        uint16_t symbol_id;
        map<string, uint16_t>* symbol_name_to_id = 0;
        vector<string>* symbol_id_to_name = 0;
        vector<verilog_symbol>* symbol_table = 0;
        string Special_Char_Set = "(){}[]:,;=";
        string  Delimeters = "\n\t\r";

    public:
    explicit verilog_Lexer(){};
        explicit verilog_Lexer(const char* file){
            Tok = new ut_Tokenizer(file, Delimeters.c_str(), Special_Char_Set.c_str());
        }
        ~verilog_Lexer(){
            delete Tok;
            cout<<"Tokenizer killed"<<endl;
        }
        void get_Token(verilog_token* v_tok);
        vector<verilog_symbol>* copy_symbol_table(){
            return symbol_table;
        }
        map<string, uint16_t>* copy_symbol_name_to_id(){
            return symbol_name_to_id;
        }
        vector<string>* copy_id_to_name(){
            return symbol_id_to_name;
        }
        void init_Module(){
            symbol_name_to_id = new map<string, uint16_t>;
            symbol_table = new vector<verilog_symbol>;
            symbol_id_to_name = new vector<string>;
        }
    
    private:
        Tokid_t get_Tok_id();
        Tokid_t handle_identifier(char* lexeme);
        Tokid_t handle_special_char(char* lexeme);
        bool handle_Integer(char* lexeme);
};



#endif