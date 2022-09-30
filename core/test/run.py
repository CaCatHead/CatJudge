#!/usr/bin/env python

# -*- coding: utf-8 -*-

import os
import sys
import shutil
import subprocess
import tempfile


def compile_source(source, tmp_dir):
    output =  os.path.join(tmp_dir, "a.out")
    commands = ["g++", source, "-o", output, "-static", "-w",
                "-lm", "-std=c++11", "-O2", "-DONLINE_JUDGE"]
    try:
        subprocess.check_output(commands, stderr=subprocess.STDOUT)
        os.chmod(output, 0o775)
        return None
    except subprocess.CalledProcessError as e:
        return e.output


def run(executable, source, testcase):
    __dir__ = os.path.dirname(__file__)
    tmp_dir = tempfile.mkdtemp()
    os.chmod(tmp_dir, 0o775)
    source_path = os.path.join(tmp_dir, source)
    shutil.copy(os.path.join(__dir__, "source", source), source_path)
    err_output = compile_source(source_path, tmp_dir)
    if err_output:
        shutil.rmtree(tmp_dir)
        return False

    shutil.copy(os.path.join(__dir__, "testcase", testcase + '.in'), os.path.join(tmp_dir, 'in.in'))
    shutil.copy(os.path.join(__dir__, "testcase", testcase + '.ans'), os.path.join(tmp_dir, 'out.out'))

    commands = [executable, "-d", tmp_dir, "-l", "2", "-s", "lcmp"]
    code = subprocess.call(commands, cwd=os.path.dirname(executable))

    def read_verdict():
        with open(os.path.join(tmp_dir, 'result.txt')) as result:
            return result.read().split('\n')[0].strip()

    verdict = read_verdict()
    shutil.rmtree(tmp_dir)

    if code != 0:
        return False

    return verdict != 'Runtime Error'


if __name__ == '__main__':
    ok = run(sys.argv[1], sys.argv[2], sys.argv[3])
    if not ok:
        exit(1)
