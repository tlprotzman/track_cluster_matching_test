import os
import subprocess
import multiprocessing
from functools import partial
from tqdm import tqdm

template = 'eicrecon --loadconfigs eicrecon.config -Ppodio:output_file=output/{particle}.root /home/tristan/epic/single_particle_sims/output/{particle}.root'
particles = {'e+', 'e-', 'mu+', 'mu-', 'pi+', 'pi-', 'kaon+', 'kaon-', 'proton', 'anti_proton'}
# particles = {'e-'}
n_workers = 10

def run_sim(particle):
    """Run a single job with the specified job ID."""
    log_file = f"logs/{particle}.log"

    with open(log_file, "w") as log:
        cmd = template.format(particle=particle).split()
        return_code = subprocess.call(cmd, stdout=log, stderr=log)
    return return_code == 0

def run_simulations():
    os.makedirs("logs", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    with multiprocessing.Pool(processes=min(n_workers, len(particles))) as pool:
        results = list(tqdm(pool.imap(run_sim, particles), total=len(particles), desc="Running simulations", unit="sim"))
    return all(results)

def run_analysis(particle):
    """Run a single job with the specified job ID."""
    log_file = f"logs/{particle}_ana.log"

    with open(log_file, "w") as log:
        cmd = f'root -q -b -x -l matching_performance.cxx("{particle}")'.split()
        return_code = subprocess.call(cmd, stdout=log, stderr=log)
    return return_code == 0

def run_analyses():
    os.makedirs("logs", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    with multiprocessing.Pool(processes=min(n_workers, len(particles))) as pool:
        results = list(tqdm(pool.imap(run_analysis, particles), total=len(particles), desc="Running analyses", unit="sim"))
    return all(results)


                          

run_simulations()
run_analyses()
