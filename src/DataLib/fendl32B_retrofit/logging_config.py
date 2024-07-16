import logging

class LineFormatter(logging.Formatter):
    """
    Custom formatter to handle log messages and enforce character limits
        in the log messages.
    """

    def format(self, record, max_length = 79):
        """
        Format a log record, optionally applying custom line splitting.

            This method overrides the default log formatting to either use a
            default format or apply custom line splitting based on the length
            of the log message. If the log record has a `use_default_format`
            attribute set to True, it returns the default formatted message
            without line splitting.

        Arguments:
            record (logging.LogRecord): The log record to be formatted.
            max_length (int, optional): The maximum allowable length of
                each line. Default is set to 79 characters, the standard
                line length by PEP 8 standards.

        Returns:
            str: The formatted log message,
                potentially with custom line splitting.
        """

        # Format the timestamp and levelname
        prefix = super().format(record)
        
        # Check if the log record has a special flag indicating different formatting
        if hasattr(record, 'use_default_format') and record.use_default_format:
            # Just return the default formatted message without line splitting
            return f"{prefix} {record.msg}"
        
        # Apply the default line-splitting formatting
        if len(prefix) + len(record.msg) > max_length:
            message_lines = self._split_message(prefix, record.msg, max_length)
            formatted_message = '\n'.join(message_lines)
            return formatted_message
        else:
            return f"{prefix} {record.msg}"
    
    def _split_message(self, prefix, message, max_length):
        """
        Split the message into lines that fit within max_length.

        Arguments: 
            prefix (str): The prefix to prepend the first line of the message.
            message (str): The message to be split into lines.
            max_length (int, optional): The maximum allowable length of
                each line. Default is set to 79 characters, the standard
                line length by PEP 8 standards.            

        Returns:
            lines (list of str): A list of lines, each fitting within 
                the specified length.
        """

        lines = []
        current_line = prefix
        
        # Split the message into lines that fit within max_length
        for word in message.split():
            if len(current_line) + len(word) + 1 <= max_length:
                current_line += ' ' + word
            else:
                lines.append(current_line)
                # For subsequent lines, do not repeat the prefix
                current_line = ' ' * len(prefix) + ' ' + word
        
        if current_line.strip():
            lines.append(current_line)
        
        return lines

class CustomFilter(logging.Filter):
    """
    Custom filter to apply different formatting based on a condition.
    """

    def filter(self, record):
        """
        Determine if a log record should use default formatting based
            on its content.

            This method checks if the log record's message contains a specific
            NJOY flag, which is a string of 77 asterisks characteristic of the
            output produced from running NJOY. If the flag is found, a custom
            attribute `use_default_format` is set to True on the record,
            indicating that the default formatting should be used. Otherwise,
            the attribute is set to False.

        Arguments:
            record (logging.LogRecord): The log record to be checked
                and potentially modified.
        
        Returns:
            bool: Always returns True, allowing the log record to be processed
                by the file handler.
        """

        njoy_flag = ('*' * 77)
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

file_handler = logging.FileHandler('fendl3_retrofit.log', mode='w')
file_handler.setLevel(logging.INFO)

formatter = LineFormatter('%(asctime)s - %(levelname)s')
file_handler.setFormatter(formatter)

custom_filter = CustomFilter()
file_handler.addFilter(custom_filter)

logger.addHandler(file_handler)
logger.propagate = False