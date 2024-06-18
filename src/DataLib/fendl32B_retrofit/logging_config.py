# Import packages
import logging

# Configure logging
logging.basicConfig(
    level= 'INFO',
    format='%(asctime)s - %(levelname)s - %(message)s',
    filename= 'fendl3_gendf.log',
    filemode= 'w'
)

logger = logging.getLogger(__name__)