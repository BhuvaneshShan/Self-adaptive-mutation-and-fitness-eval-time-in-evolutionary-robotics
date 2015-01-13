#include <assert.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include "common.h"

using namespace std;

struct GraphEntry {
    int timestep;
    double value;
    
    GraphEntry(int timestep_, double value_) : timestep(timestep_), value(value_) {}
};

void get_command_line_params(int argc, char** argv, string& in_filename, string& out_filename,
    unsigned int& granularity, unsigned int &window_size)
{
    if (argc < 5) {
        cout << "Invocation: " << argv[0] << " <in_file.cxd> <out_file.csv> <granularity> <window_size>" << endl;
        exit(1);
    }
    
    in_filename = string(argv[1]);
    out_filename = string(argv[2]);
    granularity = parse_uint(argv[3]);
    window_size = parse_uint(argv[4]);
    
    if (granularity < 1) {
        cerr << "Invalid granularity value" << endl;
        exit(1);
    }
    if (window_size < 1) {
        cerr << "Invalid window size" << endl;
        exit(1);
    }
    if (window_size % granularity) {
        cerr << "Window size must be multiple of granularity" << endl;
        exit(1);
    }
}

void get_global_fitness_graph(vector<RawExperimentDataEntry>& entries, vector<GraphEntry>& graph)
{
    int n_agents = 0;
    for (vector<RawExperimentDataEntry>::iterator entry = entries.begin();
        entry != entries.end(); entry++) {
        n_agents = max(n_agents, (int)(entry->agent_id + 1));
    }
    
    vector<double> curr_fitness(n_agents, 0.0);
    uint32_t last_timestep = 0;
    
    for (vector<RawExperimentDataEntry>::iterator entry = entries.begin();; entry++) {
        if ((entry == entries.end()) || (entry->timestep > last_timestep)) {
            double avg = kahan_sum(curr_fitness) / n_agents;
            graph.push_back(GraphEntry(last_timestep, avg));
        }
        
        curr_fitness[entry->agent_id] = entry->fitness;
        last_timestep = entry->timestep;
        
        if (entry == entries.end()) break;
    }
}

void integrate_fitness_graph(vector<GraphEntry>& graph)
{
    double integral = 0;
    double c = 0;
    int last_timestep = 0;
    double last_fitness = 0;
    
    for (vector<GraphEntry>::iterator entry = graph.begin(); entry != graph.end(); entry++) {
        double term = last_fitness * (entry->timestep - last_timestep);
        
        // We add using Kahan's algorithm to minimize floating-point error
        double y = term - c;
        double t = integral + y;
        c = (t - integral) - y;
        integral = t;
        
        last_timestep = entry->timestep;
        last_fitness = entry->value;
        
        entry->value = integral;
    }
}

void sample_fitness_integral(vector<GraphEntry>& integral, unsigned int granularity,
    vector<GraphEntry>& sampled_integral)
{
    int next_ts = 0;
    int last_timestep = 0;
    double last_value = 0;
    
    for (vector<GraphEntry>::iterator entry = integral.begin(); entry != integral.end(); entry++) {
        while (entry->timestep > next_ts) {
            double interpolated = last_value + (next_ts - last_timestep) *
                (entry->value - last_value) / (entry->timestep - last_timestep);
            sampled_integral.push_back(GraphEntry(next_ts, interpolated));
            
            next_ts += granularity;
        }
        
        last_timestep = entry->timestep;
        last_value = entry->value;
    }
}

void get_fitness_graph(vector<RawExperimentDataEntry>& entries, unsigned int granularity,
    unsigned int window_size, vector<GraphEntry>& fitness_graph)
{
    vector<GraphEntry> graph;
    get_global_fitness_graph(entries, graph);
    integrate_fitness_graph(graph);
    
    vector<GraphEntry> sampled_integral;
    sample_fitness_integral(graph, granularity, sampled_integral);
    
    unsigned int delta = window_size / granularity;
    
    for (unsigned int i = 0; i < sampled_integral.size(); i++) {
        double win_integral = sampled_integral[i].value - ((i >= delta) ?
            sampled_integral[i-delta].value : 0);
        
        fitness_graph.push_back(GraphEntry(sampled_integral[i].timestep,
                                win_integral / window_size));
    }
}

void write_fitness_graph_data(string& out_filename, vector<GraphEntry>& fitness_graph)
{
    ofstream out_file(out_filename.c_str(), ios::out);
    if (out_file.fail()) {
        cerr << "Could not open output file '" << out_filename << "'" << endl;
        exit(1);
    }
    
    out_file << "Timestep,Fitness\n";
    
    for (vector<GraphEntry>::iterator entry = fitness_graph.begin();
        entry != fitness_graph.end(); entry++) {
        out_file << entry->timestep << "," << entry->value << "\n";
    }
    
    out_file.close();
}

int main(int argc, char** argv)
{
    string in_filename, out_filename;
    unsigned int granularity, window_size;
    get_command_line_params(argc, argv, in_filename, out_filename, granularity, window_size);
    
    vector<RawExperimentDataEntry> entries;
    read_compressed_experiment_file(in_filename, entries);
    
    vector<GraphEntry> fitness_graph;
    get_fitness_graph(entries, granularity, window_size, fitness_graph);
    
    write_fitness_graph_data(out_filename, fitness_graph);
    
    return 0;
}
