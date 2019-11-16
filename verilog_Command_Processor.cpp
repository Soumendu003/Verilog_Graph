#include"Verilog_Command_Processor.h"

void verilog_Command_Processor::process_Cmd(){
    cmd_type = get_CmdType();
    
    switch (cmd_type){
    case module:
        process_module();
        break;

    case input:
        process_input();
        break;

    case output:
        process_output();
        break;

    case wire:
        process_wire();
        break;
    
    case gate:
        process_gate();
        break;
        
    case module_call:
        process_module_call();
        break;
    
    case endmodule:
        process_endmodule();
        break;

    case assign:
        process_assign();
        break;

    default:
        break;
    }
}