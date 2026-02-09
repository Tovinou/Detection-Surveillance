import datetime
import config

def log_event(message: str) -> None:
    """Appends a timestamped message to the log file."""
    try:
        with open(config.LOG_FILE, 'a') as f:
            f.write(f"{datetime.datetime.now()}: {message}\n")
    except IOError as e:
        print(f"Could not write to log file: {e}")
