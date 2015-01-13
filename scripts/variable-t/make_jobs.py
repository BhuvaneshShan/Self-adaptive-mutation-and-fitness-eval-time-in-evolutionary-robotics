#!/usr/bin/python

import os, sys, itertools, re, inspect, shutil, math, stat

ITERATIONS = 15000000
T_VALUES = ['64','128','256','512','1024','2048','evo']
EXP_TYPES = ['coldboot','sequences']
TRIALS = 30
CORES = 16

WALLTIME = None
BASE_DIR = None
EXP_BASE_DIR = None
SESSION_DIR = None

ITER_PER_HOUR = 4000000

COMMON_CFG_REPLACEMENTS = [
    (lambda k: k.endswith('Filename'), lambda v: os.path.join(BASE_DIR, v)),
    ('gDumpProperties', 'false'),
    ('gLogFilename', '/dev/null'),
    ('gFontName', '/System/Library/Fonts/LucidaGrande.ttc'),
    ('gBatchMode', 'true')
]

def hours_to_walltime(hours):
    total_sec = int(math.ceil(hours * 3600.0))
    
    hours = total_sec // 3600
    minutes = (total_sec % 3600) // 60
    seconds = total_sec % 60
    
    return '{0}:{1:02}:{2:02}'.format(hours, minutes, seconds)

def walltime_to_hours(walltime):
    match = re.match(r'(\d+)(:(\d\d))?(:(\d\d))?$', walltime)
    if not match:
        return None
    
    hours = int(match.group(1)) if match.group(1) is not None else 0
    minutes = int(match.group(3)) if match.group(3) is not None else 0
    seconds = int(match.group(5)) if match.group(5) is not None else 0
    
    if minutes > 59 or seconds > 59:
        return None
    
    total_sec = hours * 3600 + minutes * 60 + seconds
    
    return total_sec / 3600.0

def get_input(prompt, default=None, validator=None):
    if default is not None:
        prompt += ' [default={0}]'.format(default)
    
    while True:
        entry = raw_input(prompt+': ')
        if entry == "":
            entry = default
        if validator is not None and not validator(entry):
            print "Invalid input"
            continue
        return entry

def get_base_dir():
    path = os.path.abspath(sys.argv[0])
    
    while path != '/':
        path, dir = os.path.split(path)
        if dir == 'scripts':
            return path
    
    raise RuntimeError("Something is wrong - makejobs should be in /scripts "
                       "relative to the main roborobo directory!")

def get_default_session_dir():
    present = set(os.listdir(EXP_BASE_DIR))
    
    for i in itertools.count(1):
        sess_name = 'session{0}'.format(i)
        if not sess_name in present:
            return sess_name

def get_session_dir():
    default = get_default_session_dir()
    
    while True:
        dir = get_input('Enter session name', default, lambda x: len(x) > 0)
        sess_dir = os.path.join(EXP_BASE_DIR, dir)
        if os.path.exists(sess_dir):
            delete = get_input("Warning: Directory '{0}' already exists. Purge its contents? (y/n)".format(dir),
                               'n', lambda x: x in ['y','n'])
            if delete == 'n':
                continue
            
            shutil.rmtree(sess_dir, True)
        
        return sess_dir

def get_experiment_types():
    def _validate(input):
        return set(input.lower().replace(' ', '').split(',')) <= set(['coldboot','sequences'])
        
    default = ','.join(EXP_TYPES)
    
    print "  ! Supported experiment types:"
    print "  ! coldboot: Run scenarios 1-3 individually"
    print "  ! sequences: All permutations of scenarios 1-3"
    
    input = get_input('Enter experiment types', default, validator=_validate)
    
    return input.lower().replace(' ', '').split(',')

def get_iterations():
    valid = lambda x: x.isdigit() and int(x) > 1
    return int(get_input('Enter # of iterations', str(ITERATIONS), valid))

def get_tvalues():
    default = ','.join(str(t) for t in T_VALUES)
    valid = lambda x: re.match(r'^((\d+|evo),)*(\d+|evo)$', x.replace(' ',''), re.I)
    spec = get_input('Enter T-values', default, valid)
    
    return [t.strip().lower() for t in spec.split(',')]

def get_trials():
    valid = lambda x: x.isdigit() and int(x) > 1
    return int(get_input('Enter # of trials', str(TRIALS), valid))

def get_cores():
    valid = lambda x: x.isdigit() and int(x) > 1
    return int(get_input('Use nodes with how many cores?', str(CORES), valid))

def get_walltime():
    default = hours_to_walltime(math.ceil(4.0 * ITERATIONS / ITER_PER_HOUR) / 4.0)
    valid = lambda x: walltime_to_hours(x) is not None
    walltime = get_input('Maximum wall time', default, valid)

    return hours_to_walltime(walltime_to_hours(walltime))

def setup_session_dir():
    for dir in ['configs','jobs','compressed_experiment_data']:
        dir = os.path.join(SESSION_DIR, dir)
        if not os.path.isdir(dir):
            os.makedirs(dir)

def replace_value(value, replacement):
    return replacement(value) if inspect.isfunction(replacement) else replacement

def transform_config_file(out_filename, replacements):
    dict_repl = dict(item for item in replacements if not inspect.isfunction(item[0]))
    lambda_repl = [item for item in replacements if inspect.isfunction(item[0])]
    
    in_filename = os.path.join(BASE_DIR, 'config/variable-t.properties')
    with open(out_filename, "w+") as out_file:
        with open(in_filename, "r") as in_file:
            for line in in_file:
                match = re.match(r'(\s*)([^#=]+?)(\s*=\s*)([^#]*?)(\s*)(#.*)?$', line, re.DOTALL)
                if match:
                    fields = [('' if g is None else g) for g in match.groups()]
                    
                    key = fields[1]
                    value = fields[3]
                    
                    for key_fn, repl in lambda_repl:
                        if key_fn(key):
                            value = str(replace_value(value, repl))
                    
                    if dict_repl.has_key(key):
                        value = str(replace_value(value, dict_repl[key]))
                        del dict_repl[key]
                    
                    fields[3] = value
                    
                    out_file.write(''.join(fields))
                else:
                    out_file.write(line)

        for key, repl in dict_repl.items():
            value = str(replace_value(None, repl))
            out_file.write("{0} = {1}\n".format(key, value))

def gen_configs_for_programme(base_name, programme):
    for t in T_VALUES:
        for i in xrange(1, TRIALS+1):
            name_core = '{0}_t{1}_r{2}'.format(base_name, t, i)
            
            cfg_filename = os.path.join(SESSION_DIR, 'configs/config_'+name_core+'.properties')
            fitness_filename = 'fitness_'+name_core+'.log'
            
            replacements = list(COMMON_CFG_REPLACEMENTS)
            replacements.extend([
                ('gProgramme', ' '.join(str(item) for item in programme)),
                ('gFitnessLogFilename', fitness_filename),
                ('gMaxIt', ITERATIONS),
                ('evolveEvaluationTime', 'true' if t == 'evo' else 'false'),
                ('gRandomSeed', i)
            ])
            
            if t != 'evo':
                replacements.append(('evolutionStepsPerCandidate', t))
            
            for item in programme:
                replacements.append(('gSubscenario{0}Runtime'.format(item),
                                     int(math.ceil(float(ITERATIONS) / len(programme)))))
            
            transform_config_file(cfg_filename, replacements)

def generate_configs():
    print "Generating config files..."
    
    for exp_type in EXP_TYPES:
        if exp_type == 'coldboot':
            for scenario in [1,2,3]:
                gen_configs_for_programme('scen{0}'.format(scenario), [scenario])
        elif exp_type == 'sequences':
            for seq in itertools.permutations([1,2,3]):
                gen_configs_for_programme('seq{0}'.format('_'.join(str(item) for item in seq)), seq)

def generate_jobs():
    print "Generating job files..."
    
    configs_dir = os.path.join(SESSION_DIR, 'configs')
    configs = os.listdir(configs_dir)

    n_jobs = 0
    for batch in (configs[i:i+CORES] for i in xrange(0, len(configs), CORES)):
        n_jobs += 1
        job_filename = os.path.join(SESSION_DIR, 'jobs/batch{0}.sh'.format(n_jobs))
        
        with open(job_filename, 'w+') as f:
            f.write("#PBS -lwalltime={0}\n".format(WALLTIME))
            f.write("#PBS -lnodes=1:cores{0}\n".format(CORES))
            f.write("#PBS -S /bin/bash\n")
            f.write("cd $TMPDIR\n")
            
            for cfg_file in batch:
                esc = lambda x : x.replace(' ', "\\ ")
                
                basename = 'fitness_' + cfg_file[len('config_'):-len('.properties')]
                
                roborobo_exec = os.path.join(BASE_DIR, 'roborobo')
                config_file = os.path.join(configs_dir, cfg_file)
                result_file = os.path.join(SESSION_DIR, 'compressed_experiment_data', basename+'.cxd')
                scripts_dir = os.path.join(BASE_DIR, 'scripts', 'variable-t', 'analysis')
                binarize_exec = os.path.join(scripts_dir, 'binarize_experiment_data')
                compress_exec = os.path.join(scripts_dir, 'compress_experiment_data')
                
                commands = []
                commands.append(esc(roborobo_exec) + " -l " + esc(config_file))
                commands.append(esc(binarize_exec) + " {0}.log {0}.bin".format(basename))
                commands.append(esc(compress_exec) + " {0}.bin {0}.cxd".format(basename))
                commands.append('cp {0}.cxd '.format(basename) + esc(result_file))
                
                f.write("(" + " && ".join(commands) + ") & \n")
            
            f.write("wait\n")
    
    return n_jobs

def copy_session_scripts():
    script_dir = os.path.join(BASE_DIR, 'scripts/variable-t/session')
    
    for f in os.listdir(script_dir):
        src = os.path.join(script_dir, f)
        if os.path.isfile(src):
            dest = os.path.join(SESSION_DIR, f)
            shutil.copy(src, dest)
            os.chmod(dest, stat.S_IRWXU + stat.S_IRWXG + stat.S_IRWXO)


try:
    BASE_DIR = get_base_dir()
    EXP_BASE_DIR = os.path.join(BASE_DIR, 'experiments/variable-t')
    SESSION_DIR = get_session_dir()
    EXP_TYPES = get_experiment_types()
    ITERATIONS = get_iterations()
    T_VALUES = get_tvalues()
    TRIALS = get_trials()
    CORES = get_cores()
    WALLTIME = get_walltime()
    
    setup_session_dir()
    generate_configs()
    n_jobs = generate_jobs()
    copy_session_scripts()
    
    wall_hrs = walltime_to_hours(WALLTIME)
    
    print "Done"
    print
    print "{0} nodes x {1} hours = {2} node-hours".format(n_jobs, wall_hrs, n_jobs * wall_hrs)
    print
    print "You can now cd to '{0}' and start the jobs using 'run.sh'".format(os.path.split(SESSION_DIR)[1])
except RuntimeError as e:
    sys.stderr.write(str(e)+"\n")
    exit(1)
