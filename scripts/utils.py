import sys

class ColoredPrint:
  HEADER = "\033[95m"
  OKBLUE = "\033[94m"
  OKCYAN = "\033[96m"
  OKGREEN = "\033[92m"
  WARNING = "\033[93m"
  ERROR = "\033[91m"
  ENDC = "\033[0m"

  @classmethod
  def printi(cls, msg: str, output_file=sys.stderr):
    cls.__colored_print(msg, cls.OKBLUE, "Info: ", output_file)

  @classmethod
  def printw(cls, msg: str, output_file=sys.stderr):
    cls.__colored_print(msg, cls.WARNING, "Warning: ", output_file)

  @classmethod
  def printe(cls, msg: str, output_file=sys.stderr):
    cls.__colored_print(msg, cls.ERROR, "ERROR: ", output_file)

  @classmethod
  def __colored_print(cls, msg: str, style: str, header: str, output_file):
    print(f"{style}{header}{msg}{cls.ENDC}", file=output_file)