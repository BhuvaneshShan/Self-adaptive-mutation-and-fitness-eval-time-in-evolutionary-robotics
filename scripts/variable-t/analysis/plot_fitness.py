#!/usr/bin/python

from optparse import OptionParser

import math, os, sys, subprocess, itertools, re, json

GRAPH_STYLES = [
                {'color': '#000000', 'gray_color': '#000000', 'point_type': 0, 'width': 3 },
                {'color': '#ff0000', 'gray_color': '#808080', 'point_type': 6 },
                {'color': '#ff8000', 'gray_color': '#c0c0c0', 'point_type': 8 },
                {'color': '#ffff00', 'gray_color': '#000000', 'point_type': 7 },
                {'color': '#00c000', 'gray_color': '#a0a0a0', 'point_type': 2 },
                {'color': '#0000ff', 'gray_color': '#404040', 'point_type': 9 },
                {'color': '#ff00ff', 'gray_color': '#000000', 'point_type': 4 }
                ]

UNSPIKE_BUFFER = 150000

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

def get_graphs(data, omit):
    graphs = []
    
    for i, t in enumerate(data['taus']):
        if str(t) in omit:
            continue
        
        x_s = sorted(data[t].keys())
        
        graphs.append({ 'name': "T={0}".format(t),
                        'with' : 'linespoints',
                        'style': GRAPH_STYLES[0 if t=='evo' else 1+i],
                        'x_s' : x_s,
                        'y_s' : [avg(data[t][x]) for x in x_s] })
    
    return graphs

def normalize_graphs(graphs, normalization):
    for graph in graphs:
        new_x_s = []
        new_y_s = []
        
        for x, y in zip(graph['x_s'], graph['y_s']):
            for from_x, to_x, ceiling in normalization:
                if from_x > 0 and x > from_x and x - from_x < UNSPIKE_BUFFER:
                    y = None
                    break
                if x >= from_x and x <= to_x:
                    y = float(y) / ceiling
                    break
            
            if y is not None:
                new_x_s.append(x)
                new_y_s.append(y)
        
        graph['x_s'] = new_x_s
        graph['y_s'] = new_y_s

def setup_normalization(fig_name, max_x, options):
    if not fig_name.startswith('seq'):
        return None # Not a sequence
    if options.normalization_params is None:
        return None # Normalization not configured
    
    np = options.normalization_params
    
    sequence = fig_name[3:].split('_')
    
    normalization = []
    for i, season in enumerate(sequence):
        if season not in np:
            raise RuntimeError("No info for season {0} found in normalization parameters".format(season))
        
        from_x = int(i * max_x / len(sequence))
        to_x = int((i+1) * max_x / len(sequence)) 
        normalization.append((from_x, to_x, np[season]['max_fitness']))
    
    return normalization

def process_figure(fig_name, files, out_filename, options):
    X_SCALE = 1000000.0
    
    print "Plotting figure '{0}'...".format(out_filename)
    
    data = gather_figure_data(files)
    graphs = get_graphs(data, options.omit)
    
    max_x = max((max(graph['x_s']) for graph in graphs))
    
    normalization = setup_normalization(fig_name, max_x, options)
    if normalization is not None:
        normalize_graphs(graphs, normalization)
    
    with open("temp.gplot", "w+") as f:
        f.write('set term postscript eps solid size 8.5cm,6cm\n')
        f.write('set output "| epstopdf --filter --outfile={0}"\n'.format(out_filename))
        f.write('set termoption dash\n')
        f.write('set tmargin 0.5\n')
        f.write('set rmargin 0.5\n')
        f.write('set lmargin 7.5\n')
        
        f.write('set xlabel "# of iterations"\n')
        f.write('set xrange [0:{0}]\n'.format(max_x / X_SCALE))
        if normalization is None:
            f.write('set ylabel "Average fitness" offset 1\n')
        else:
            f.write('set ylabel "Normalized fitness" offset 1\n')
            f.write('set yrange [0:1.1]\n')
        
        f.write('set format x "%0.1fM"\n')
        f.write('set format y "%0.2f"\n')
        
        f.write('set key right bottom Left nobox\n')

        if normalization is not None:
            for _, to_x, _ in normalization:
                f.write('set arrow from {0},0.0 to {0},1.1 lt 3 nohead\n'.format(to_x / X_SCALE))

        f.write('f(x)=1\n')
        f.write('plot \\\n')
        
        if normalization is not None:
            f.write("    f(x) notitle with lines lt 3,\\\n")
        
        for g in graphs:
            style = g['style']
            
            if not options.grayscale:
                params = "with lines lc rgb '{0}'".format(style['color'])
            else:
                params = "with linespoints lc rgb '{0}' pt {1} pi 120" \
                    .format(style['gray_color'], style['point_type'])
            
            params += ' lt 1'
            if 'width' in style:
                params += ' lw {0}'.format(style['width'])
            
            f.write('    "-" title "{0}" {1}'.format(g['name'], params))
            if g != graphs[-1]:
                f.write(",\\")
            f.write("\n")
        
        for g in graphs:
            for x, y in zip(g['x_s'], g['y_s']):
                f.write("    {0} {1}\n".format(x / X_SCALE, y))
            f.write("end\n")
        
    subprocess.call(['gnuplot', 'temp.gplot'])
    
    os.remove('temp.gplot')

def process(in_dirs, out_dir, options):
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
    
    figures = dict()
    
    for in_dir in in_dirs:
        for f in os.listdir(in_dir):
            match = re.match(r'fitness_graph_(.*)_t(\d+|evo)_r\d+$', os.path.splitext(f)[0])
            if match:
                fig_name = match.group(1)
                if not fig_name in figures:
                    figures[fig_name] = list()
                figures[fig_name].append(os.path.join(in_dir, f))
    
    for fig_name, files in figures.items():
        out_filename = os.path.join(out_dir, 'fitness_' + fig_name + '.pdf')
        process_figure(fig_name, files, out_filename, options)

def parse_args():
    parser = OptionParser("Usage: %prog [options] <in_directory1> [<in_directory2> ...] <out_directory>")
    parser.add_option("-g", "--grayscale",
                      action="store_true", dest="grayscale", default=False,
                      help="Activates grayscale mode")
    parser.add_option("-n", "--normalization-file", metavar="<file.json>", dest="normalization_file",
                      help="Normalization data for sequence graphs")
    parser.add_option("-o", "--omit", metavar="<tau_list>", dest="omit",
                      help="Skip these traces (e.g.'128,1024,evo')")
    (options, args) = parser.parse_args()
    
    if options.normalization_file is not None:
        with open(options.normalization_file) as f:
            options.normalization_params = json.load(f)
    else:
        options.normalization_params = None
    
    if options.omit is not None:
        options.omit = set([item.strip() for item in options.omit.split(',')])
    else:
        options.omit = set()
    
    if len(args) < 2:
        parser.error("Not enough arguments provided")

    in_dirs = args[:-1]
    out_dir = args[-1]
    
    for in_dir in in_dirs:
        if not os.path.isdir(in_dir):
            raise RuntimeError("Input directory '{0}' does not exist".format(in_dir))
    
    return (in_dirs, out_dir, options)


try:
    in_dirs, out_dir, options = parse_args()
    
    process(in_dirs, out_dir, options)
except RuntimeError as e:
    sys.stderr.write(str(e)+"\n")
    exit(1)
