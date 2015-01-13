#!/usr/bin/python

import sys, os, subprocess, pwd, re, time, random

from subprocess import Popen, PIPE

DEFAULT_GRANULARITY = 10000
DEFAULT_WINDOW_SIZE = 100000
DEFAULT_CORES = 16
DEFAULT_NODES = 10

BASE_DIR = None
IS_LISA = False

class Task:
    name = None
    commands = None
    out_files = None
    
    def __init__(self, name):
        self.name = name
        self.commands = []
        self.out_files = []

def recon_system():
    out, _ = Popen(['which', 'qstat'], stdout=PIPE, stderr=PIPE).communicate()
    
    IS_LISA = (out != '')

def get_base_dir():
    path = os.path.abspath(sys.argv[0])
    
    while path != '/':
        path, dir = os.path.split(path)
        if dir == 'experiments':
            return path
    
    raise RuntimeError("Something is wrong - on_completion should be in /experiments "
                       "relative to the main roborobo directory!")

def jobs_still_running():
    if not IS_LISA:
        return False
    
    username = pwd.getpwuid(os.getuid()).pw_name

    out, err = Popen(['qstat', '-u', username], stdout=PIPE, stderr=PIPE).communicate()

    return out != ''

def move_job_outputs():
    files = [f for f in os.listdir('.') if re.search(r'[.](e|o)\d+$', f)]
    if len(files) == 0:
        return

    if not os.path.exists('stdout'):
        os.makedirs('stdout')

    for src in files:
       dest = os.path.join('stdout', src)
       os.rename(src, dest)

    print "Moved job outputs to 'stdout' directory"

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

def get_command():
    print
    print "Select operation:"
    print "0. Exit"
    print "1. Report on job times and failures"
    print "2. Extract fitness graph data"
    print "3. Render fitness plots"
    print
    
    valid = lambda x: x in ['0', '1', '2', '3']
    cmd = int(get_input('Enter operation', validator=valid))
    print
    
    return cmd

def show_jobs_report():
    meta = {}
    
    for f in os.listdir('stdout'):
        batch_name, ext = os.path.splitext(f)
        job_no = ext[2:]
        
        if not job_no in meta:
            meta[job_no] = dict([('name', batch_name)])
        
        meta[job_no]['out_file' if ext[1] == 'o' else 'err_file'] = os.path.join('stdout', f)
    
    report = []
    for job_no, job in meta.items():
        if os.stat(job['err_file']).st_size == 0:
            msg = 'OK'
            with open(job['out_file'], 'r') as f:
                for line in f:
                    match = re.search(r'vmem=[^,]*,walltime=(\d\d:\d\d:\d\d)', line)
                    if match:
                        msg = 'OK, walltime={0}'.format(match.group(1))
                        break
        else:
            with open(job['err_file'], 'r') as f:
                msg = "ERROR: {0}".format(f.readline().strip())
        
        msg = 'Job #{0} ({1}) - {2}'.format(job_no, job['name'], msg)
        
        report.append((job['name'], msg))
    
    report.sort(key = lambda x : x[0])
    
    print "\n".join(item[1] for item in report)

def shell_esc(filename):
    return filename.replace(' ', '\\ ')

def run_processing_jobs_on_lisa(tasks):
    valid = lambda x: x.isdigit() and int(x) > 1
    nodes = int(get_input('Use how many nodes', str(DEFAULT_NODES), valid))
    cores = int(get_input('Use how many cores per node', str(DEFAULT_CORES), valid))
    
    core_dist = [list() for _ in xrange(nodes*cores)]
    core_idx = 0
    random.shuffle(tasks)
    for task in tasks:
        core_dist[core_idx].append(task)
        core_idx += 1
        if core_idx == len(core_dist): core_idx = 0

    jobs = []
    for machine in [core_dist[mach_no*cores:(mach_no+1)*cores] for mach_no in xrange(nodes)]:
        threads = []
        for thread_id, thread_tasks in enumerate(machine):
            operations = []
            for task in thread_tasks:
                for command in task.commands:
                    repl = lambda m : "out_{0}_{1}.out".format(thread_id, m.group(1))
                    command = re.sub(r'%out(\d+)', repl, command)
                    
                    operations.append(command)
                
                for i, out_file in enumerate(task.out_files):
                    operations.append("cp out_{0}_{1}.out {2}".format(
                        thread_id, i, shell_esc(out_file)))
            
            threads.append(' && '.join(operations))
        
        if len(threads) == 0:
            continue
        
        job_name = 'temp_processing_job_{0}.sh'.format(1+len(jobs))
        jobs.append(job_name)
        with open(job_name, 'w+') as f:
            f.write("#PBS -lwalltime=00:30:00\n")
            f.write("#PBS -lnodes=1:cores{0}\n".format(cores))
            f.write("#PBS -S /bin/bash\n")
            f.write("cd $TMPDIR\n")
            f.write("(" + ") &\n(".join(threads) + ") &\n")
            f.write("wait\n")
    
    print
    print "Qsub'ing processing jobs..."
    for job in jobs:
        subprocess.call(['qsub', job])
    
    print "Waiting for jobs to complete..."
    while True:
        time.sleep(5)
        if not jobs_still_running():
            break
    
    for file in os.listdir('.'):
        if file.startswith('temp_processing_job_'):
            os.remove(file)

def run_processing_jobs_locally(tasks):
    with open("temp_jobs.sh", 'w+') as f:
        for i, task in enumerate(tasks):
            for command in task.commands:
                repl = lambda m : shell_esc(task.out_files[int(m.group(1))])
                command = re.sub(r'%out(\d+)', repl, command)
            
            f.write('echo "{0}/{1}: {2}..."\n'.format(i+1, len(tasks), task.name))
            f.write(command + "\n")
    
    subprocess.call(['bash', './temp_jobs.sh'])
    
def run_processing_jobs(tasks):
    if IS_LISA:
        run_processing_jobs_on_lisa(tasks)
    else:
        run_processing_jobs_locally(tasks)

def process_fitness():
    base_dir = get_base_dir()
    session_dir = os.path.split(os.path.abspath(sys.argv[0]))[0]
    
    valid = lambda x: x.isdigit() and int(x) > 1
    granularity = int(get_input('Enter granularity', str(DEFAULT_GRANULARITY), valid))
    window_size = int(get_input('Enter window size', str(DEFAULT_WINDOW_SIZE), valid))
    if (window_size % granularity):
        print "ERROR: Window size must be multiple of granularity"
        return
    
    for d1 in ['fixed-t', 'evo-t']:
        for d2 in ['coldboot', 'sequences']:
            dir = 'fitness_graph_data/' + d1 + '/' + d2
            if not os.path.isdir(dir):
                os.makedirs(dir)
    
    tasks = []
    files = os.listdir('compressed_experiment_data')
    for f in files:
        task = Task(f)
        
        script_filename = os.path.join(base_dir, 'scripts/variable-t/analysis/extract_fitness_graph_data')
        in_file = os.path.join(session_dir, 'compressed_experiment_data/'+f)
        
        out_basename = os.path.splitext(f.replace('fitness_', 'fitness_graph_'))[0] + '.csv'
        out_file = os.path.join(session_dir, 'fitness_graph_data',
                                'evo-t' if '_tevo_' in f else 'fixed-t', out_basename)
        
        task.commands.append('{0} {1} %out0 {2} {3}'.format(
            shell_esc(script_filename), shell_esc(in_file), granularity, window_size))
        task.out_files = [out_file]
        
        tasks.append(task)
    
    run_processing_jobs(tasks)
    
    print "Done! Results available in 'fitness_graph_data'"

def plot_fitness():
    base_dir = get_base_dir()
    scripts_dir = os.path.join(base_dir, 'scripts/variable-t/analysis')
    session_dir = os.path.split(os.path.abspath(sys.argv[0]))[0]
    
    print "Computing normalization params..."
    
    script = os.path.join(scripts_dir, 'extract_normalization_params.py')
    input_dir = os.path.join(session_dir, 'fitness_graph_data/fixed-t/coldboot')
    norm_file = os.path.join(session_dir, 'normalization_params.json')
    subprocess.call(['python', script, input_dir, norm_file])
    
    print "Plotting graphs..."
    
    script = os.path.join(scripts_dir, 'plot_fitness.py')
    input_dirs = []
    for d1 in ['fixed-t', 'evo-t']:
        for d2 in ['coldboot', 'sequences']:
            input_dirs.append(os.path.join(session_dir, 'fitness_graph_data', d1, d2))
    
    for variant in ['full_color', 'for_paper']:
        output_dir = os.path.join(session_dir, 'fitness_plots', variant)
        args = ['python', script, '-n', norm_file]
        if variant == 'for_paper':
            args.extend(['-g', '-o128,1024'])
        args.extend(input_dirs)
        args.extend([output_dir])
        subprocess.call(args)
    
    print "Done! Results available in 'fitness_plots'"


try:
    recon_system()
    
    if jobs_still_running():
        raise RuntimeError("You still have jobs running - you may run this program only when all jobs are complete")

    move_job_outputs()
    
    while True:
        cmd = get_command()
        if cmd == 0:
            break
        elif cmd == 1:
            show_jobs_report()
        elif cmd == 2:
            process_fitness()
        elif cmd == 3:
            plot_fitness()

except RuntimeError as e:
    sys.stderr.write(str(e)+"\n")
    exit(1)
