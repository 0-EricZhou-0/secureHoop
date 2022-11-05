#!/usr/bin/python3

# build gem5.opt -- /usr/bin/env python3 $(which scons) build/X86/gem5.opt -j 16

import os, re, subprocess, sys, shlex
from typing import Callable
from utils import ColoredPrint as cp

script_path = os.path.dirname(os.path.abspath(__file__))

class Config:
  gem5_base_path = os.path.join(script_path, os.path.pardir)
  # dramsim3_base_path = os.path.join(gem5_base_path, "ext/dramsim3/DRAMsim3")
  # dramsim3_config_path = os.path.join(dramsim3_base_path, "configs")

  benchmark_base_path = os.path.join(gem5_base_path, "benchmarks")
  se_path = os.path.join(gem5_base_path, "configs/example/se.py")
  stat_summary_file = os.path.join(gem5_base_path, "out")

  ARCHS = ["ARM", "NULL", "MIPS", "POWER", "RISCV", "SPARC", "X86"]
  BUILD_VARIENTS = ["debug", "opt", "fast"]

  def __init__(self, arch: str, build_version: str):
    if arch not in Config.ARCHS:
      cp.printe(f"Architecture <{arch}> not supported by gem5")
      exit(1)
    if build_version not in Config.BUILD_VARIENTS:
      cp.printe(f"Build version <{build_version}> not supported by gem5")
      exit(1)
    self.__arch = arch
    self.__build_version = build_version
    self.__gem5_executable =  os.path.join(Config.gem5_base_path,
        "build", self.__arch, f"gem5.{self.__build_version}")
    if not os.path.isfile(self.__gem5_executable):
      cp.printe(f"Gem5 executable for <{self.__arch}, {self.__build_version}> \
          is not found at path <{os.path.abspath(self.__gem5_executable)}>, \
          check if the executable is compiled")
      exit(1)
    self.__gem5_options = []
    self.__se_options = []

  def set_cpu_param(self, cpu_type: str, num_cpu: int, clk_feq: float):
    self.__cpu_type = cpu_type
    self.__num_cpu = num_cpu
    self.__clk_feq = clk_feq

  def set_extra_options(self, option: str, se_option: bool=True):
    for sub_option in option.split():
      if not sub_option.startswith("--"):
        cp.printw(f"Option <{sub_option}> might be ill formatted")
      if se_option:
        self.__se_options.append(sub_option)
      else:
        self.__gem5_options.append(sub_option)

  def set_output_dir(self, relative_path: str, create_if_missing: bool=False):
    output_dir_full = os.path.join(Config.gem5_base_path, relative_path)
    if not os.path.isdir(output_dir_full):
      if create_if_missing:
        subprocess.run(["mkdir", "-p", output_dir_full])
      else:
        cp.printe(f"Output directory <{os.path.abspath(output_dir_full)}> does not exist")
        exit(1)
    self.__output_dir = output_dir_full

  def simulate(self, benchmark: str, argument_list: str="",
      filters: list=[".*"], truncate_output: bool=False, gdb: bool=False):
    benchmark_path_full = os.path.join(Config.benchmark_base_path, benchmark)
    if not os.path.isfile(benchmark_path_full):
        cp.printe(f"Benchmark <{os.path.abspath(benchmark_path_full)}> does not exist")
        exit(1)

    command = f"{'gdb --args ' if gdb else ''}{self.__gem5_executable} \
        {' '.join(self.__gem5_options)} --outdir={self.__output_dir} \
        {Config.se_path} --cpu-type={self.__cpu_type} \
        --num-cpus={self.__num_cpu} --cpu-clock={self.__clk_feq}GHz \
        {' '.join(self.__se_options)} --cmd={benchmark_path_full} \
        --options=\"{argument_list}\""
    capture = not truncate_output
    process = subprocess.Popen(shlex.split(command), stderr=subprocess.PIPE)
    for line in iter(process.stderr.readline, b""):
      if truncate_output and line.startswith(b"#CAPTURING_START"):
        capture = True
      if capture:
        to_print = line.decode()
        for filter in filters:
          if re.match(filter, to_print):
            sys.stdout.write(to_print)
            break
      if truncate_output and line.startswith(b"#CAPTURING_END"):
        capture = False

  def get_stat(self, pattern_method: dict):
    result = { pattern : [pattern_method[pattern], [], set(), 0]
        for pattern in pattern_method.keys() }
    with open(os.path.join(self.__output_dir, "stats.txt")) as f:
      for line in f.readlines():
        preprocess = line.split()
        if len(preprocess) < 2:
          continue
        stat, val, *_ = preprocess
        for pattern in pattern_method.keys():
          if re.match(pattern, stat):
            result[pattern][1].append(float(val))
            result[pattern][2].add(stat)
    for pattern, stat_func in pattern_method.items():
      result[pattern][3] = stat_func(result[pattern][1])
    return result

c = Config("X86", "opt")
c.set_cpu_param("X86O3CPU", 4, 4)
# cache
c.set_extra_options("--caches --l2cache --l3cache --cacheline_size=64 \
    --l1i_size=32kB --l1i_assoc=8 --l1d_size=32kB --l1d_assoc=8 \
    --l2_size=2MB --l2_assoc=16 --l3_size=6MB --l3_assoc=24 \
    --maccache --mac_size=4096B --mac_assoc=64 \
    --metcache --met_size=128kB --met_assoc=8 ")
# nvm
c.set_extra_options(f"--mem-type=NVM_2400_1x64 --mem-size=8GB")
# debug
# c.set_extra_options(f"--debug-flags=Cache", se_option=False)
# simulation
c.set_output_dir("output/test", create_if_missing=True)
c.simulate("precompiled/FFT", "-p4 -m16", truncate_output=False, gdb=False)
# c.simulate("precompiled/FFT", "-p2 -m16", truncate_output=False, gdb=False,
#     filters=[".*l2cache.*", ".*blk.*", ".*WB2.*"])
# c.simulate("test/main", truncate_output=False, gdb=False)
# c.simulate("test/main", truncate_output=False, gdb=False,
#     filters=".*\.l2cache.*")

# pattern_method = { "simSeconds": sum, "simTicks": sum }
# result = c.get_stat(output_dir, pattern_method)
# print(result)
