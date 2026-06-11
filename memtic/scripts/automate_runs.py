import os
import subprocess
import json
import shutil

# Use absolute-style paths relative to the script location for better reliability
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SOLVER_EXEC = os.path.join(SCRIPT_DIR, "../uctp_solver")
INPUT_DIR = os.path.join(SCRIPT_DIR, "../inputs/ITC-2002")
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "../outputs")
TIMETABLES_DIR = os.path.join(SCRIPT_DIR, "../timetables")
DATASETS = ["small_1", "small_2", "small_3", "small_4", "small_5"]

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    for dataset in DATASETS:
        print(f"\n🚀 STARTING RUN FOR: {dataset}")
        input_path = os.path.join(INPUT_DIR, f"{dataset}.dat")

        # 1. Execute Solver
        subprocess.run([SOLVER_EXEC, input_path])

        # 2. Get latest folder
        subfolders = [f.path for f in os.scandir(TIMETABLES_DIR) if f.is_dir()]
        if not subfolders:
            continue
        latest_folder = sorted(subfolders)[-1]

        # 3. Find absolute best (HCV=0)
        best_scv = float('inf')
        best_json_path = None

        for file in os.listdir(latest_folder):
            if file.endswith("_timetable.json"):
                with open(os.path.join(latest_folder, file), 'r') as f:
                    data = json.load(f)
                    if data['fitness']['hcv'] == 0 and data['fitness']['scv'] < best_scv:
                        best_scv = data['fitness']['scv']
                        best_json_path = os.path.join(latest_folder, file)

        # 4. Copy best result
        if best_json_path:
            dest_path = os.path.join(OUTPUT_DIR, f"run_{dataset.replace('_', '')}.json")
            shutil.copy(best_json_path, dest_path)
            print(f"✅ SUCCESS: Saved SCV {best_scv} to {dest_path}")

    # 5. Run Plotter (Using the correct path now)
    plotter_script = os.path.join(SCRIPT_DIR, "main.py")
    if os.path.exists(plotter_script):
        print("\n📊 Generating convergence chart...")
        subprocess.run(["python3", plotter_script])

if __name__ == "__main__":
    main()