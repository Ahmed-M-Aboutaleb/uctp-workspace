import logging
import os
from logging.handlers import RotatingFileHandler

# Define where logs will live inside the container
LOG_DIR = "/app/logs"

# Ensure the directory exists when the app starts
os.makedirs(LOG_DIR, exist_ok=True)

def get_logger(module_name: str) -> logging.Logger:
    """
    Returns a custom logger that outputs to both the console and a rotating file.
    """
    logger = logging.getLogger(module_name)
    
    # Prevent duplicate handlers if the logger is imported multiple times
    if logger.hasHandlers():
        return logger

    logger.setLevel(logging.INFO)

    # Define the format of the log messages
    formatter = logging.Formatter(
        "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
    )

    # 1. Console Handler (for 'docker logs' compatibility)
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)

    # 2. File Handler (Rotates when file reaches 5MB, keeps the last 5 backups)
    log_file_path = os.path.join(LOG_DIR, "uctp_system.log")
    file_handler = RotatingFileHandler(
        log_file_path, 
        maxBytes=5 * 1024 * 1024, # 5 MB
        backupCount=5
    )
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)

    return logger