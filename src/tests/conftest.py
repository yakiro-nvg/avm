import os, sys, shutil
import subprocess
from glob import glob

PATH = os.path.dirname(__file__)
COV_DIR = os.path.join(PATH, '../.gcov')
MERGED_COV_DIR = os.path.join(PATH, '../.merged_gcov')
AVM_DIR = os.path.normpath(os.path.join(PATH, '../..'))
AVM_INC = os.path.normpath(os.path.join(PATH, '../inc'))
AVM_PRI = os.path.normpath(os.path.join(PATH, '../private'))

if os.path.exists(MERGED_COV_DIR):
    shutil.rmtree(MERGED_COV_DIR)
os.mkdir(MERGED_COV_DIR)

def zero_uncovered_line(line):
    s, l, c = line.split(":", 2)
    s = s.strip(); l = l.strip()
    if s[0] == '#': s = '0'
    return '%s:%s:%s' % (s, l, c)

def merge_line(pair):
    s0, l0, c0 = pair[0].split(":", 2)
    s1, l1, c1 = pair[1].split(":", 2)
    assert(l0 == l1 and c0 == c1)
    if s0 == '-':
        assert(s1 == '-')
        return '%s:%s:%s' % (s0, l0, c0)
    else:
        return '%s:%s:%s' % (int(s0) + int(s1), l0, c0)

def merge(path):
    with open(path, 'r') as f:
        lines = f.read().splitlines()
        source = [x.strip() for x in lines[0].split(":", 2)][2][7:]
        if source.startswith(AVM_INC) or source.startswith(AVM_PRI):
            relative = os.path.relpath(source, AVM_DIR)
            lines = ['-:0:Source:' + relative] + map(zero_uncovered_line, lines[5:])
            name = os.path.basename(path)
            merged_path = os.path.join(MERGED_COV_DIR, name)
            if os.path.exists(merged_path):
                with open(merged_path, 'r') as mf:
                    mf_lines = mf.read().splitlines()
                    assert(len(lines) == len(mf_lines))
                    lines = map(merge_line, zip(lines, mf_lines))
            with open(merged_path, 'w') as mf:
                mf.write('\n'.join(lines))

def gcov(f):
    cwd = os.getcwd()
    if os.path.exists(COV_DIR):
        shutil.rmtree(COV_DIR)
    os.mkdir(COV_DIR)
    os.chdir(COV_DIR)
    command = ["gcov", f]
    p = subprocess.Popen(
        command,
        stdout = subprocess.PIPE,
        stderr = subprocess.STDOUT)
    p.wait()
    os.chdir(cwd)
    messages = p.stdout.read()
    if p.returncode != 0:
        raise Exception("Couldn't run gcov properly\n{}".format(messages))
    for i in glob(os.path.join(COV_DIR, '*.gcov')):
        merge(i)

def pytest_configure():
    sys.path.append(os.path.join(PATH, '../..'))
    sys.path.append(os.path.join(PATH, '..'))

def pytest_sessionfinish(session, exitstatus):
    if exitstatus == 0:
        for path in glob(os.path.join(PATH, '../_ffi_*.gcda')):
            gcov(path)
        if os.path.exists(COV_DIR):
            shutil.rmtree(COV_DIR)
