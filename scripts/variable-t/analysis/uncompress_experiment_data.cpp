#include <assert.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include "common.h"

using namespace std;

void get_command_line_params(int argc, char** argv, string& in_filename, string& out_filename)
{
    if (argc < 3) {
        cout << "Invocation: " << argv[0] << " <in_file.cxd> <out_file.bin>" << endl;
        exit(1);
    }
    
    in_filename = string(argv[1]);
    out_filename = string(argv[2]);
}

int main(int argc, char** argv)
{
    string in_filename, out_filename;
    get_command_line_params(argc, argv, in_filename, out_filename);
    
    vector<RawExperimentDataEntry> entries;
    read_compressed_experiment_file(in_filename, entries);
    write_binary_experiment_file(out_filename, entries);
    
    return 0;
}
