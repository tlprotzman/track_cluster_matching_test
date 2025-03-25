import os
import subprocess
import multiprocessing
from functools import partial
from tqdm import tqdm
import time
import re

template = '''
npsim -N {events} --compactFile {DETECTOR_PATH}/{DETECTOR_CONFIG}.xml \
--outputFile {outfile} \
--enableGun  --gun.particle {particle} \
--gun.thetaMin 45*deg --gun.thetaMax 135*deg --gun.distribution cos(theta) \
--gun.momentumMin {pt_min}*GeV --gun.momentumMax {pt_max}*GeV
'''

particles = {'e+', 'e-', 'mu+', 'mu-', 'pi+', 'pi-', 'kaon+', 'kaon-', 'proton', 'anti_proton'}
pt_min = 0.1
pt_max = 10
n_events = 25000
n_workers = 14
detector_path = os.environ["DETECTOR_PATH"]
detector_config = os.environ["DETECTOR_CONFIG"]

def get_last_event(filename):
    pattern = re.compile(r'Initializing event (\d+)')
    # Start with reading small blocks from the end
    with open(filename, 'rb') as f:
        f.seek(0, 2)  # Go to end of file
        chunk_size = 8192
        position = f.tell()
        
        # Read chunks backwards until match is found
        while position > 0:
            # Move position back by chunk size
            position = max(0, position - chunk_size)
            f.seek(position)
            chunk = f.read(min(chunk_size, f.tell())).decode('utf-8', errors='ignore')
            
            # Find all matches in this chunk
            matches = list(pattern.finditer(chunk))
            if matches:
                return int(matches[-1].group(1))
        
    return 0


def run_sim(particle):
    """Run a single job with the specified job ID."""
    log_file = f"logs/{particle}.log"
    out_file = f"output/{particle}.root"
    
    with open(log_file, "w") as log:
        # Adjust this command based on how your simulation should be run
        cmd = template.format(DETECTOR_PATH=detector_path, DETECTOR_CONFIG=detector_config, events=n_events, outfile=out_file, particle=particle, pt_min=pt_min, pt_max=pt_max).split()
        return_code = subprocess.call(cmd, stdout=log, stderr=log)
    return return_code == 0

def run_simulations():
    started = False
    with tqdm(total=len(particles) * n_events, desc="Events generated") as pbar:
        with multiprocessing.Pool(processes=min(n_workers, len(particles))) as pool:
            results = [pool.apply_async(run_sim, (particle,)) for particle in particles]
            while not all(result.ready() for result in results):
                processed_events = 0
                for particle in particles:
                    log_file = f"logs/{particle}.log"
                    if os.path.exists(log_file):
                        processed_events += get_last_event(log_file)
                        if not started and processed_events > 0:
                            started = True
                            pbar.unpause()

                pbar.n = processed_events
                pbar.refresh()
                time.sleep(1)
            pool.close()
            pool.join()
                          

run_simulations()