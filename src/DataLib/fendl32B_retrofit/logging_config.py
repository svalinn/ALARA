# Import packages
import logging
import textwrap
from reaction_data import establish_directory

class CustomFormatter(logging.Formatter):
    """
    Custom formatter to handle log messages and enforce character limits.
    """

    def format(self, record):
        """
        Format a log record, applying line splitting based on message length.

        Arguments:
            record (logging.LogRecord): The log record to be formatted.

        Returns:
            str: The formatted log message.
        """

        # Format the timestamp and levelname
        prefix = super().format(record)
        
        # Check if the record has a special flag indicating different format
        if hasattr(record, 'use_default_format') and record.use_default_format:
            # Just return the default formatted message without line splitting
            return f'{prefix} {record.msg}\n'
        
        message_lines = textwrap.fill(record.msg, initial_indent=prefix + ' ',
                                     subsequent_indent=' '*len(prefix) + ' ',
                                     width=79, break_long_words=False)
        
        return f'{message_lines}\n'

class CustomFilter(logging.Filter):
    """
    Custom filter to apply different formatting based on a condition.
    """

    def filter(self, record):
        """
        Determine if a log record should use default formatting based on its
            content.

        Arguments:
            record (logging.LogRecord): The log record to be checked and
                potentially modified.

        Returns:
            bool: Always returns True, allowing the log record to be processed
                by the file handler.
        """

        njoy_flag = '*' * 77
        if njoy_flag in record.msg:
            # Set a flag in the record to use default formatting
            record.use_default_format = True
        else:
            # Ensure default formatting is applied for other messages
            record.use_default_format = False
        return True

# Configure logger
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# Ensure that logger is saved in the same directory as the mt_table.csv file
dir = establish_directory()
file_handler = logging.FileHandler(f'{dir}/fendl3_retrofit.log', mode='w')
file_handler.setLevel(logging.INFO)

formatter = CustomFormatter('%(asctime)s - %(levelname)s')
file_handler.setFormatter(formatter)

custom_filter = CustomFilter()
file_handler.addFilter(custom_filter)

logger.addHandler(file_handler)
logger.propagate = False