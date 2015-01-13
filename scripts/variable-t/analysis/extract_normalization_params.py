#!/usr/bin/python

from optparse import OptionParser

import math, os, sys, subprocess, itertools, re, json

def read_datafile(filename, field_types, separator=','):
    with open(filename, "r") as f:
        f.readline()
        
        for line in f:
            line = line.strip()
            item = []
            for i, field in enumerate(line.split(separator)):
                item.append(field_types[i](field))
            
            yield item

def create_file(filename):
    path, _ = os.path.split(filename)
    if path != '' and not os.path.isdir(path):
        os.makedirs(path)
    
    f = open(filename, 'w+')
    
    return f

def avg(values):
    return math.fsum(values) / len(values) if len(values) > 0 else None

def stddev(values):
    if len(values) == 0: return None
    
    mean = avg(values)
    var = math.fsum([(v-mean)*(v-mean) for v in values]) / len(values)
    
    return math.sqrt(var)

def params_from_filename(filename):
    match = re.match(r'.*_t(\d+|evo)_r(\d+)$', os.path.splitext(os.path.split(filename)[1])[0])
    
    return (match.group(1), int(match.group(2)))

def gather_figure_data(files):
    data = dict()
    all_params = set([params_from_filename(f) for f in files])
    key_fn = lambda t: sys.maxint if t == 'evo' else int(t)
    data['taus'] = taus = sorted(list(set(params[0] for params in all_params)), key=key_fn)
    data['reps'] = reps = range(1, 1+max(params[1] for params in all_params))
    
    missing = set((t, r) for t in taus for r in reps)
    
    for f in files:
        t, r = params_from_filename(f)
        missing.discard((t, r))
        
        if not t in data:
            data[t] = dict()
        
        for timestep, fitness in read_datafile(f, [int,float]):
            if timestep not in data[t]:
                data[t][timestep] = list()
            data[t][timestep].append(fitness)
    
    if len(missing) > 0:
        t, r = missing.pop()
        raise RuntimeError("Data for curve T={0}, rep={1} is missing!".format(t, r))

    return data

def get_max_fitness(data):
    max_fitness = 0
    for t in data['taus']:
        max_fitness = max(max_fitness, max([avg(y_s) for y_s in data[t].values()]))
    
    return max_fitness

def get_normalization_params(files):
    data = gather_figure_data(files)
    
    return { 'max_fitness': get_max_fitness(data) }

def process(in_dirs, out_file):
    seasons = dict()
    
    for in_dir in in_dirs:
        for f in os.listdir(in_dir):
            match = re.match(r'fitness_graph_scen(\d+)_t\d+_r\d+$', os.path.splitext(f)[0])
            if match:
                season_id = match.group(1)
                if not season_id in seasons:
                    seasons[season_id] = list()
                seasons[season_id].append(os.path.join(in_dir, f))
    
    for season_id in seasons:
        seasons[season_id] = get_normalization_params(seasons[season_id])
    
    with create_file(out_file) as f:
        json.dump(seasons, f, indent = 4);
        
def parse_args():
    parser = OptionParser("Usage: %prog <in_directory1> [<in_directory2> ...] <out_file.json>")
    (_, args) = parser.parse_args()
    
    if len(args) < 2:
        parser.error("Not enough arguments provided")

    in_dirs = args[:-1]
    out_file = args[-1]
    
    for in_dir in in_dirs:
        if not os.path.isdir(in_dir):
            raise RuntimeError("Input directory '{0}' does not exist".format(in_dir))
    
    return (in_dirs, out_file)


try:
    in_dirs, out_file = parse_args()
    
    process(in_dirs, out_file)
except RuntimeError as e:
    sys.stderr.write(str(e)+"\n")
    exit(1)
