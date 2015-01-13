#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <inttypes.h>

using namespace std;

struct RawExperimentDataEntry {
    uint32_t agent_id;
    uint32_t timestep;
    float fitness;
    uint32_t tau;
} __attribute__((packed));

struct CompressedExperimentDataHeader {
    uint32_t n_agents;
} __attribute__((packed));

struct CompressedExperimentDataAgentHeader {
    uint32_t agent_id;
    uint32_t common_tau;
    uint32_t extra_time;
    uint32_t n_entries;
} __attribute__((packed));

unsigned int parse_uint(const char* str);
unsigned int parse_uint(string& str);

double kahan_sum(double* terms, int n_terms);
double kahan_sum(vector<double>& terms);

void read_binary_experiment_file(string& in_filename, RawExperimentDataEntry*& entries, int& n_entries);
void read_compressed_experiment_file(string& in_filename, vector<RawExperimentDataEntry>& entries);

void write_binary_experiment_file(string& out_filename, RawExperimentDataEntry* entries, int n_entries);
void write_binary_experiment_file(string& out_filename, vector<RawExperimentDataEntry>& entries);

#endif
