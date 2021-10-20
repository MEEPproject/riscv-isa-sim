#include <map>
#include <string>

//The latencies for vector instructions are for executions with enough lanes to consume the whole vector length in parallel
extern std::map<std::string, int> insn_to_latency;
