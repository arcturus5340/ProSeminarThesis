import subprocess
import os


flags = {
    'native&parallel': ['-fopenmp', '-fopenmp-simd', '-ftree-vectorize', '-ffast-math', '-lmvec', '-march=native', '-mfma', '-O3'],
    'native': ['-fopenmp-simd', '-ftree-vectorize', '-ffast-math', '-lmvec', '-march=native', '-mfma', '-O3'],
    'avx512': ['-fopenmp-simd', '-ftree-vectorize', '-ffast-math', '-lmvec', '-mavx512f', '-mfma', '-O3'],
    'avx2': ['-fopenmp-simd', '-ftree-vectorize', '-ffast-math', '-lmvec', '-mavx2', '-mfma', '-O3'],
    'sse4.2': ['-fopenmp-simd', '-ftree-vectorize', '-ffast-math', '-lmvec', '-msse4.2', '-mfma', '-O3'],
    'mmx': ['-fopenmp-simd', '-ftree-vectorize', '-ffast-math', '-lmvec', '-mmmx', '-mfma', '-O3'],
    'autovec': ['-ftree-vectorize', '-ffast-math', '-lmvec', '-mmmx', '-mfma', '-O3'],
    'none': ['-O2'],
}

optimized = {
    'native&parallel': 2,
    'native': 1,
    'avx512': 1,
    'avx2': 1,
    'sse4.2': 1,
    'mmx': 1,
    'autovec': 0,
    'none': 0,
}


print(f'{"flags" :>15}  {"min" :>10} {"average" :>10} {"max" :>10}')
print('PAGE RANK')
for key in  optimized.keys():
    average_time = 0
    min_time = float('inf')
    max_time = 0
    if optimized[key] == 2:
        file = 'page_rank/paralled.cpp'
    elif optimized[key] == 1:
        file = 'page_rank/vectorised.cpp'
    else:
        file = 'page_rank/unoptimised.cpp'
    for i in range(30):
        os.system(f'g++ {" ".join(flags[key])} -o a.out {file}')
        program_output = subprocess.check_output('./a.out', shell=True)
        program_execution_time = int(program_output.split()[3])
        average_time += program_execution_time
        max_time = max(max_time, program_execution_time)
        min_time = min(min_time, program_execution_time)
        os.system('rm ./a.out')

    print(f'{key :>15}: {min_time :>10} {average_time / 30 :>10} {max_time :>10} [µs]')

print('PERLIN NOISE')
for key in  optimized.keys():
    average_time = 0
    min_time = float('inf')
    max_time = 0
    if optimized[key] == 2:
        file = 'perlin_noise/paralled.cpp'
    elif optimized[key] == 1:
        file = 'perlin_noise/vectorised.cpp'
    else:
        file = 'perlin_noise/unoptimised.cpp'
    for i in range(30):
        os.system(f'g++ {" ".join(flags[key])} -o a.out {file}')
        program_output = subprocess.check_output('./a.out', shell=True)
        program_execution_time = int(program_output.split()[3])
        average_time += program_execution_time
        max_time = max(max_time, program_execution_time)
        min_time = min(min_time, program_execution_time)
        os.system('rm ./a.out')

    print(f'{key :>15}: {min_time :>10} {average_time / 30 :>10} {max_time :>10} [µs]')
