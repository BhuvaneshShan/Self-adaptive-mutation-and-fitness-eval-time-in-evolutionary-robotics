#include <iostream>
#include <fstream>
#include <cstdlib>

#include "common.h"

using namespace std;

void get_command_line_params(int argc, char** argv, string& in_filename, string& out_filename)
{
    if (argc < 3) {
        cout << "Invocation: " << argv[0] << " <in_file.bin> <out_file.log>" << endl;
        exit(1);
    }
    
    in_filename = string(argv[1]);
    out_filename = string(argv[2]);
}

void write_text_experiment_file(string& out_filename, RawExperimentDataEntry* entries, int n_entries)
{
    ofstream out_file(out_filename.c_str(), ios::out);
    if (out_file.fail()) {
        cerr << "Could not open output file '" << out_filename << "'" << endl;
        exit(1);
    }
    
    out_file << "Id,Timestep,Fitness,EvaluationTime\n";
    
    for (int i = 0; i < n_entries; i++) {
        out_file << entries[i].agent_id << "," << entries[i].timestep << ","
            << entries[i].fitness << "," << entries[i].tau << "\n";
    }
    
    out_file.close();
}

int main(int argc, char** argv)
{
    string in_filename, out_filename;
    get_command_line_params(argc, argv, in_filename, out_filename);
    
    RawExperimentDataEntry* entries;
    int n_entries;
    read_binary_experiment_file(in_filename, entries, n_entries);
    write_text_experiment_file(out_filename, entries, n_entries);
    
    return 0;
}
