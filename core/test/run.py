#!/usr/bin/env python

# -*- coding: utf-8 -*-

import os
import sys
import stat
import shutil
import subprocess
import tempfile


def compile_source(source, tmp_dir):
    is_c_cpp = source.endswith('.c') or source.endswith('.cpp')

    output = os.path.join(tmp_dir, "a.out" if is_c_cpp else "Main.class")
    commands = ["g++", source, "-o", output, "-static", "-w",
                "-lm", "-std=c++11", "-O2", "-DONLINE_JUDGE"] if is_c_cpp else ["javac", "Main.java", "-d", "."]

    try:
        subprocess.check_output(commands, cwd=tmp_dir, stderr=subprocess.STDOUT)
        os.chmod(output, 0o775)
        return None
    except subprocess.CalledProcessError as e:
        return e.output


def run(executable, checker, source, testcase, expected):
    __dir__ = os.path.dirname(__file__)
    tmp_dir = tempfile.mkdtemp()
    os.chmod(tmp_dir, 0o775)
    source_path = os.path.join(tmp_dir, source if not source.endswith('.java') else 'Main.java')
    shutil.copy(os.path.join(__dir__, "source", source), source_path)
    err_output = compile_source(source_path, tmp_dir)
    if err_output:
        print(err_output)
        shutil.rmtree(tmp_dir)
        return False

    shutil.copy(os.path.join(__dir__, "testcase", testcase + '.in'), os.path.join(tmp_dir, 'in.in'))
    shutil.copy(os.path.join(__dir__, "testcase", testcase + '.ans'), os.path.join(tmp_dir, 'out.out'))
    os.chmod(os.path.join(__dir__, "testcase", testcase + '.in'), stat.S_IRWXU)
    os.chmod(os.path.join(__dir__, "testcase", testcase + '.ans'), stat.S_IRWXU)

    is_c_cpp = source.endswith('.c') or source.endswith('.cpp')
    commands = [executable, "-d", tmp_dir, "-l", source.split('.')[-1], "-s", checker]
    if is_c_cpp:
        # Restrict c/cpp memory usage
        commands += ["-m", str(4 * 1024)]
        
    code = subprocess.call(commands, cwd=os.path.dirname(executable))

    def read_verdict():
        with open(os.path.join(tmp_dir, 'result.txt')) as result:
            content = result.read()
            first_line = content.split('\n')[0]
            return first_line.split()[1].strip()

    verdict = read_verdict()
    shutil.rmtree(tmp_dir)

    if code != 0:
        print("Something went wrong when running catj")
        return False

    if verdict == expected:
        return True
    else:
        print('Expected verdict: "' + expected + '", but find "' + verdict + '"')
        return False


if __name__ == '__main__':
    ok = run(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
    if not ok:
        exit(1)
