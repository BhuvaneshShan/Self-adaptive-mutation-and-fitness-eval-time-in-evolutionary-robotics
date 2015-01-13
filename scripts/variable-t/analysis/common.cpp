#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "common.h"

using namespace std;

unsigned int parse_uint(const char* str)
{
    if (strspn(str, "0123456789") != strlen(str)) {
        cerr << "'" << str << "' is not a valid unsigned number" << endl;
        exit(1);
    }
    
    unsigned int result;
    sscanf(str, "%u", &result);
    
    return result;
}

unsigned int parse_uint(string& str)
{
    return parse_uint(str.c_str());
}

double kahan_sum(double* terms, int n_terms)
{
    double sum = 0.0;
    double c = 0.0;
    
    for (int i = 0; i < n_terms; i++) {
        double y = terms[i] - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    
    return sum;
}

double kahan_sum(vector<double>& terms)
{
    return kahan_sum(&terms[0], terms.size());
}

void read_binary_experiment_file(string& in_filename, RawExperimentDataEntry*& entries, int& n_entries)
{
    ifstream in_file(in_filename.c_str(), ios::in | ios::binary);
    if (in_file.fail()) {
        cerr << "Could not open input file '" << in_filename << "'" << endl;
        exit(1);
    }
    
    in_file.seekg(0, ios::end);
    size_t size = in_file.tellg();
    
    if ((size == 0) || (size % sizeof(RawExperimentDataEntry))) {
        cerr << "Input file not the right size" << endl;
        exit(1);
    }
    
    n_entries = size / sizeof(RawExperimentDataEntry);
    entries = new RawExperimentDataEntry[n_entries];
    
    in_file.seekg(0, ios::beg);
    in_file.read((char*)entries, sizeof(RawExperimentDataEntry) * n_entries);
    in_file.close();
}

static bool compare_by_timestep(RawExperimentDataEntry e1, RawExperimentDataEntry e2)
{
    return e1.timestep < e2.timestep;
}

void read_compressed_experiment_file(string& in_filename, vector<RawExperimentDataEntry>& entries)
{
    ifstream in_file(in_filename.c_str(), ios::in | ios::binary);
    if (in_file.fail()) {
        cerr << "Could not open input file '" << in_filename << "'" << endl;
        exit(1);
    }
    
    CompressedExperimentDataHeader header;
    
    in_file.read((char*)&header, sizeof(header));
    if (in_file.fail()) {
        cerr << "Input file '" << in_filename << "' is corrupt" << endl;
        exit(1);
    }
    
    for (unsigned int ag = 0; ag < header.n_agents; ag++) {
        CompressedExperimentDataAgentHeader ag_header;
        
        in_file.read((char*)&ag_header, sizeof(ag_header));
        if (in_file.fail()) {
            cerr << "Input file '" << in_filename << "' is corrupt" << endl;
            exit(1);
        }
        
        int n_entries = ag_header.n_entries;
        
        float* fitness_values = new float[n_entries];
        uint16_t* tau_values = (ag_header.common_tau == 0) ? new uint16_t[n_entries] : NULL;
        
        in_file.read((char*)fitness_values, sizeof(float) * n_entries);
        if (tau_values)
            in_file.read((char*)tau_values, sizeof(uint16_t) * n_entries);
        
        if (in_file.fail()) {
            cerr << "Input file '" << in_filename << "' is corrupt" << endl;
            exit(1);
        }
        
        for (int i = 0; i < n_entries; i++) {
            RawExperimentDataEntry entry;
            
            entry.agent_id = ag_header.agent_id;
            entry.fitness = fitness_values[i];
            entry.tau = tau_values ? tau_values[i] : ag_header.common_tau;
            entry.timestep = (i == 0) ? (1 + entry.tau) :
                (entries.back().timestep + entry.tau + ag_header.extra_time);
            
            entries.push_back(entry);
        }
        
        delete[] fitness_values;
        if (tau_values) delete[] tau_values;
    }
    
    in_file.close();
    
    stable_sort(entries.begin(), entries.end(), compare_by_timestep);
}

void write_binary_experiment_file(string& out_filename, RawExperimentDataEntry* entries, int n_entries)
{
    ofstream out_file(out_filename.c_str(), ios::out | ios::binary);
    if (out_file.fail()) {
        cerr << "Could not open output file '" << out_filename << "'" << endl;
        exit(1);
    }
    
    out_file.write((char*)entries, sizeof(RawExperimentDataEntry) * n_entries);
    out_file.close();
}

void write_binary_experiment_file(string& out_filename, vector<RawExperimentDataEntry>& entries)
{
    write_binary_experiment_file(out_filename, &entries[0], entries.size());
}
