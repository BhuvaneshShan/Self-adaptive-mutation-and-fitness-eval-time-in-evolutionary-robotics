#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>

#include "common.h"

using namespace std;

void get_command_line_params(int argc, char** argv, string& in_filename, string& out_filename)
{
    if (argc < 3) {
        cout << "Invocation: " << argv[0] << " <in_file.log> <out_file.bin>" << endl;
        exit(1);
    }
    
    in_filename = string(argv[1]);
    out_filename = string(argv[2]);
}

void read_text_experiment_file(string& in_filename, vector<RawExperimentDataEntry>& entries)
{
    const char* EXPECTED_HEADER = "Id,Timestep,Fitness,EvaluationTime";
    
    ifstream in_file(in_filename.c_str());
    if (in_file.fail()) {
        cerr << "Could not open input file '" << in_filename << "'" << endl;
        exit(1);
    }
    
    string line;
    getline(in_file, line);
    
    if (line != EXPECTED_HEADER) {
        cerr << "Expected header: '" << EXPECTED_HEADER << "'" << endl;
        exit(1);
    }
    
    while (true) {
        getline(in_file, line);
        if (line == "") break;
        
        RawExperimentDataEntry entry;
        if (sscanf(line.c_str(), "%u,%u,%f,%u",
            &entry.agent_id, &entry.timestep, &entry.fitness, &entry.tau) < 4) {
            cerr << "Invalid format in line: '" << line << "'" << endl;
        }
        
        entries.push_back(entry);
    }
    
    in_file.close();
}

int main(int argc, char** argv)
{
    string in_filename, out_filename;
    get_command_line_params(argc, argv, in_filename, out_filename);
    
    vector<RawExperimentDataEntry> entries;
    read_text_experiment_file(in_filename, entries);
    write_binary_experiment_file(out_filename, entries);
    
    return 0;
}
