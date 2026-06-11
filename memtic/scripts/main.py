import json
import os
import matplotlib.pyplot as plt

# Correctly point to the outputs folder
# If this script is in /scripts, ../outputs is correct.
base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../outputs"))

datasets = {
    "Small 1": "run_small1.json",
    "Small 2": "run_small2.json",
    "Small 3": "run_small3.json",
    "Small 4": "run_small4.json",
    "Small 5": "run_small5.json"
}

# Aesthetic improvements
plt.style.use('seaborn-v0_8-muted') # Cleaner color palette
plt.rcParams.update({
    'font.size': 12,
    'axes.grid': True,
    'grid.alpha': 0.3
})

plt.figure(figsize=(10, 6), dpi=300)

found_files = 0
for label, filename in datasets.items():
    file_path = os.path.join(base_dir, filename)
    try:
        with open(file_path, 'r') as f:
            data = json.load(f)
            history = data.get("convergence_history", [])
            if history:
                plt.plot(history, label=label, linewidth=2)
                found_files += 1
    except FileNotFoundError:
        print(f"Skipping: {filename} not found in {base_dir}")

if found_files > 0:
    plt.xlabel('Generation Number', fontweight='bold')
    plt.ylabel('Soft Constraint Violations (SCV)', fontweight='bold')
    plt.title('UCTP Solver Convergence Analysis', fontsize=16, pad=20)
    
    # Place legend outside to avoid covering data
    plt.legend(loc='upper right', frameon=True, shadow=True)
    
    # Optional: uncomment if the fitness range is massive
    # plt.yscale('log') 

    plt.tight_layout()
    save_path = os.path.join(base_dir, 'convergence_chart_olc.png')
    plt.savefig(save_path, dpi=300)
    print(f"✅ Chart saved to: {save_path}")
else:
    print("❌ No data files were found. Check your paths!")