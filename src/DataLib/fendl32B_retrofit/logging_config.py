# Import packages
import logging
import sys

# Configure logging
logging.basicConfig(
    level= 'INFO',
    format= '%(asctime)s - %(levelname)s - %(message)s',
    filename= 'fendl3_gendf.log',
    filemode= 'w'
)

logger = logging.getLogger(__name__)

# Redirect stdout and stderr to the logger
class LoggerWriter:
    def __init__(self, level):
        self.level = level

    def write(self, message):
        if message.strip():  # Avoid logging empty messages
            self.level(message.strip())

    def flush(self):
        pass  # No need to flush, but method must be defined

sys.stdout = LoggerWriter(logger.info)
sys.stderr = LoggerWriter(logger.error)