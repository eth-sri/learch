import os
import subprocess

def replay(prog, replay_script, ktest):
    replay_args = (replay_script, prog, ktest)
    popen = subprocess.Popen(replay_args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        popen.wait(timeout=30)
    except:
        popen.terminate()

def init_gcov(d):
    for path, _, files in os.walk(d):
        for f in files:
            if f.endswith('.gcda'):
                os.remove(os.path.join(path, f))

def run_gcov(p):
    subprocess.call(['gcov', '-b', '-c', '-i', '-l', p], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=os.path.dirname(p))

def get_gcov(d):
    to_delete = set()
    for path, _, files in os.walk(d):
        for f in files:
            if f.endswith('.gcov'): to_delete.add(os.path.join(path, f))

    for f in to_delete:
        os.remove(f)

    for path, _, files in os.walk(d):
        for f in files:
            if f.endswith('.gcda'): run_gcov(os.path.join(path, f))

    cov_lines = set()
    for path, _, files in os.walk(d):
        for f in files:
            if not f.endswith('.gcov'): continue

            with open(os.path.join(path, f)) as f_gcov:
                for l in f_gcov.readlines():
                    if l.startswith('file:'):
                        filename = os.path.basename(l[5:].strip())
                    if l.startswith('lcount:'):
                        l = l[7:].strip()
                        line, count = l.strip().split(',')
                        if int(count) > 0:
                            cov_lines.add((filename, int(line)))

    return cov_lines

def total_gcov(d):
    to_delete = set()
    for path, _, files in os.walk(d):
        for f in files:
            if f.endswith('.gcov'): to_delete.add(os.path.join(path, f))

    for f in to_delete:
        os.remove(f)

    for path, _, files in os.walk(d):
        for f in files:
            if not f.endswith('.gcno'): continue
            run_gcov(os.path.join(path, f))

    all_lines = set()
    for path, _, files in os.walk(d):
        for f in files:
            if not f.endswith('.gcov'): continue

            with open(os.path.join(path, f)) as f_gcov:
                for l in f_gcov.readlines():
                    if l.startswith('file:'):
                        filename = os.path.basename(l[5:].strip())
                    if l.startswith('lcount:'):
                        l = l[7:].strip()
                        line, _ = l.strip().split(',')
                        all_lines.add((filename, int(line)))

    return all_lines

init_cov = init_gcov
get_cov = get_gcov
total = total_gcov
