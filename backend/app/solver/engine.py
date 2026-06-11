import subprocess
import os
from pathlib import Path
from app.infrastructure.logger import get_logger

logger = get_logger(__name__)

BASE_DIR = Path(__file__).resolve().parent

class SolverEngine:
    def __init__(self):
        # Assuming your compiled C++ binary is placed here. 
        # You will need to ensure this binary is copied into the Docker container!
        self.executable_path = "/usr/local/bin/uctp_solver"
        self.output_dir = str(BASE_DIR / "workspace" / "outputs")
        
        os.makedirs(self.output_dir, exist_ok=True)

    def execute(self, input_filepath: str) -> str:
        """
        Runs the C++ solver executable in a subprocess.
        Returns the path to the generated output table.
        """
        filename = Path(input_filepath).stem
        run_output_dir = os.path.join(self.output_dir, f"{filename}_results")
        os.makedirs(run_output_dir, exist_ok=True)
        # Build the command just like you would type it in the terminal
        # Example: ./uctp_solver --input inputs/data.tim --output outputs/data_result.csv
        command = [
            self.executable_path,
            "--input", input_filepath,
            "--output", run_output_dir
        ]

        logger.info(f"Starting C++ Solver: {' '.join(command)}")

        try:
            # Run the command and wait for it to finish. 
            # capture_output grabs any std::cout or std::cerr from your C++ code
            result = subprocess.run(
                command, 
                capture_output=True, 
                text=True, 
                check=True # Raises an exception if C++ returns a non-zero exit code
            )
            
            logger.info("✅ C++ Solver completed successfully.")
            logger.debug(f"Solver Output:\n{result.stdout}")
            
            # Ensure the C++ program actually created the file
            summary_file = os.path.join(run_output_dir, "final_summary.json")
            if not os.path.exists(summary_file):
                raise FileNotFoundError("Solver completed, but output file was not found.")

            return run_output_dir

        except subprocess.CalledProcessError as e:
            # If your C++ code crashes (e.g., segfault, out of bounds)
            logger.error(f"❌ C++ Solver crashed with exit code {e.returncode}")
            logger.error(f"C++ Error Log:\n{e.stderr}")
            raise RuntimeError(f"Solver execution failed: {e.stderr}")