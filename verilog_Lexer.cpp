#include"verilog_Lexer.h"

void verilog_Lexer::get_Token(verilog_token* v_tok){
    v_tok->tok_id = get_Tok_id();
    v_tok->symbol_id = symbol_id;
    //cout<<"Token id = "<<to_string(v_tok->tok_id)<<endl;
}

Tokid_t verilog_Lexer::handle_special_char(char* lexeme){
    if(Strcmpi(lexeme, ",") == 0){
        return Tokid_t::comma;
    }
    if(Strcmpi(lexeme, ";") == 0){
        return Tokid_t::semi_colon;
    }
    if(Strcmpi(lexeme, ":") == 0){
        return Tokid_t::colon;
    }
    if(Strcmpi(lexeme, "(") == 0){
        return Tokid_t::lp;
    }
    if(Strcmpi(lexeme,")") == 0){
        return Tokid_t::rp;
    }
    if(Strcmpi(lexeme,"{") == 0){
        return Tokid_t::lc;
    }
    if(Strcmpi(lexeme,"}") == 0){
        return Tokid_t::rc;
    }
    if(Strcmpi(lexeme,"[") == 0){
        return Tokid_t::lb;
    }
    if(Strcmpi(lexeme,"]") == 0){
        return Tokid_t::rb;
    }
    if(Strcmpi(lexeme,"=") == 0){
        return Tokid_t::equal;
    }
}

bool verilog_Lexer::handle_Integer(char* lexeme){
    uint16_t len = strlen(lexeme);
    if(len == 0){
        return false;
    }
    for(uint16_t i = 0; i < len; i++){
        if(!isdigit(lexeme[i])){
            return false;
        }
    }
    symbol_id = atoi(lexeme);
    return true;
}

Tokid_t verilog_Lexer::get_Tok_id(){
    char* lexeme = Tok->get_token();
    //cout<<"lexeme = "<<string(lexeme)<<endl;
    symbol_id = -1;
    if(lexeme != 0){
        if(strchr(Special_Char_Set.c_str(), lexeme[0]) != 0){
            //cout<<"Special char found"<<endl;
            return handle_special_char(lexeme);
        }
        if(V_keywords.find(string(lexeme)) != V_keywords.end()){
            symbol_id = V_keywords.find(string(lexeme))->second;
            //cout<<"Keyword found"<<endl;
            return Tokid_t::keyword;
        }
        if(handle_Integer(lexeme)){
            //cout<<"Found Integer. value = "<<to_string(symbol_id)<<endl;
            return Tokid_t::integer;
        }
        else{
            //cout<<"Identifier found"<<endl;
            return handle_identifier(lexeme);
        }
    }
    else{
        //cout<<"EOF found"<<endl;
        return Tokid_t::v_eof;
    }
}

Tokid_t verilog_Lexer::handle_identifier(char* lexeme){
    string var = string(lexeme);
    if(strlen(lexeme) > 1 && lexeme[0] == '\\' && lexeme[1] == 'n'){
        var += string(Tok->get_token());    // gets '['
        var += string(Tok->get_token());    // gets digit
        var += string(Tok->get_token());    // gets ']'
    }
    
    if(symbol_name_to_id->find(var) != symbol_name_to_id->end()){
        symbol_id = symbol_name_to_id->find(var)->second;
    }
    else{
        symbol_name_to_id->insert({var, symbol_table->size()});
        symbol_id_to_name->push_back(var);
        verilog_symbol tem_symbol;
        tem_symbol.symbol_type = vSymbol_t::unknown;
        tem_symbol.expandable_size = 0;
        tem_symbol.base_resolve_id = 0;
        symbol_id = symbol_table->size();
        symbol_table->push_back(tem_symbol);
    }
    //cout<<"Identifier = "<<var<<endl;
    return Tokid_t::identifier;
}

