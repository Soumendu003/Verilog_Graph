#include"verilog_HyperGraph_Builder.h"

int main(){

    /*verilog_HyperGraph_Builder HG_Builder("cf_fft_256_8.v", "cf_fft_256_8.gates");
    HG_Builder.init_Builder("cf_fft_256_8");
    try{
        HG_Builder.build_HGraph();
    }
    catch(HyperGraph_Exception e){
        cout<<string(e.what())<<endl;
    }*/

    /*verilog_HyperGraph_Builder HG_Builder2("tiny_aes_256_8.v", "tiny_aes_256_8.gates");
    HG_Builder2.init_Builder("aes_128");
    try{
        HG_Builder2.build_HGraph();
    }
    catch(HyperGraph_Exception e){
        cout<<string(e.what())<<endl;
    }*/

    verilog_HyperGraph_Builder HG_Builder("cf_ldpc_256_8.v", "cf_ldpc_256_8.gates");
    HG_Builder.init_Builder("cf_ldpc");
    try{
        HG_Builder.build_HGraph();
    }
    catch(HyperGraph_Exception e){
        cout<<string(e.what())<<endl;
    }
    return 0;
}