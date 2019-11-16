#ifndef VERILOG_COMMAND_PROCESSOR
#define VERILOG_COMMAND_PROCESSOR

enum CommandType_t{
    module,
    input,
    output,
    wire,
    module_call,
    gate,
    endmodule,
    assign,
    ignore
};

class verilog_Command_Processor
{
    private:
        const char* file;
        CommandType_t cmd_type;

    public:
        explicit verilog_Command_Processor(){};
        ~verilog_Command_Processor(){};
        void process_Cmd();
    
    public:
        virtual void run_Cmd_Processor() = 0;
        virtual CommandType_t get_CmdType() = 0;
        virtual void process_module() = 0;
        virtual void process_input() = 0;
        virtual void process_output() = 0;
        virtual void process_wire() = 0;
        virtual void process_gate() = 0;
        virtual void process_endmodule() = 0;
        virtual void process_module_call() = 0;
        virtual void process_assign() = 0;
};

#endif