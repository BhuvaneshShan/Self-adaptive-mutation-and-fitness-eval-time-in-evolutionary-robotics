#include <assert.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include "common.h"

using namespace std;

struct AgentData {
    uint32_t agent_id;
    uint32_t common_tau;
    uint32_t extra_time;
    vector<float> fitness_values;
    vector<uint16_t> tau_values;
};

void get_command_line_params(int argc, char** argv, string& in_filename, string& out_filename)
{
    if (argc < 3) {
        cout << "Invocation: " << argv[0] << " <in_file.bin> <out_file.cxd>" << endl;
        exit(1);
    }
    
    in_filename = string(argv[1]);
    out_filename = string(argv[2]);
}

void process_for_agent(RawExperimentDataEntry* entries, int n_entries, AgentData& agent_data)
{
    agent_data.agent_id = entries[0].agent_id;
    
    agent_data.common_tau = entries[0].tau;
    for (int i=1; i < n_entries; i++)
        if (entries[i].tau != entries[i-1].tau) {
            agent_data.common_tau = 0;
        }
    
    assert(entries[0].timestep == entries[0].tau + 1);
    
    agent_data.extra_time = 0;
    if (n_entries > 0) {
        agent_data.extra_time = (entries[1].timestep - entries[0].timestep) - entries[1].tau;
        for (int i=2; i < n_entries; i++) {
            assert((entries[i].timestep - entries[i-1].timestep) == entries[i].tau + agent_data.extra_time);
        }
    }
    
    for (int i=0; i < n_entries; i++) {
        agent_data.fitness_values.push_back(entries[i].fitness);
        if (!agent_data.common_tau) {
            assert(entries[i].tau < 65536);
            agent_data.tau_values.push_back(entries[i].tau);
        }
    }
}

bool compare_by_agent_id(RawExperimentDataEntry e1, RawExperimentDataEntry e2)
{
    return e1.agent_id < e2.agent_id;
}

void process_by_agent(RawExperimentDataEntry* entries, int n_entries, vector<AgentData>& agents_data)
{
    stable_sort(entries, entries + n_entries, compare_by_agent_id);
    
    int last_start = 0;
    for (int i = 1; i <= n_entries; i++) {
        if ((i == n_entries) || (entries[i].agent_id != entries[last_start].agent_id)) {
            AgentData agent_data;
            process_for_agent(entries + last_start, i - last_start, agent_data);
            agents_data.push_back(agent_data);
            last_start = i;
        }
    }
}

void write_compressed_experiment_file(string& out_filename, vector<AgentData>& agents_data)
{
    ofstream out_file(out_filename.c_str(), ios::out | ios::binary);
    if (out_file.fail()) {
        cerr << "Could not open output file '" << out_filename << "'" << endl;
        exit(1);
    }
    
    CompressedExperimentDataHeader header;
    header.n_agents = agents_data.size();
    
    out_file.write((char*)&header, sizeof(header));
    
    for (vector<AgentData>::iterator agent = agents_data.begin(); agent != agents_data.end(); agent++) {
        CompressedExperimentDataAgentHeader ag_header;
        
        ag_header.agent_id = agent->agent_id;
        ag_header.common_tau = agent->common_tau;
        ag_header.extra_time = agent->extra_time;
        ag_header.n_entries = agent->fitness_values.size();
        
        out_file.write((char*)&ag_header, sizeof(ag_header));
        out_file.write((char*)&agent->fitness_values[0], sizeof(float) * ag_header.n_entries);
        if (!agent->common_tau)
            out_file.write((char*)&agent->tau_values[0], sizeof(uint16_t) * ag_header.n_entries);
    }

    out_file.close();
}

int main(int argc, char** argv)
{
    string in_filename, out_filename;
    get_command_line_params(argc, argv, in_filename, out_filename);
    
    RawExperimentDataEntry *entries;
    int n_entries;
    read_binary_experiment_file(in_filename, entries, n_entries);
    
    vector<AgentData> agents_data;
    process_by_agent(entries, n_entries, agents_data);
    
    write_compressed_experiment_file(out_filename, agents_data);
    
    return 0;
}
