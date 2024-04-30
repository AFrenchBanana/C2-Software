"""
content handler for loading different types of files into
"""
from toml import load

class TomlFiles:
    """loads a toml file and returns the value as self.data"""
    def __init__(self, filename):
        self.__fn = open(filename,"rt", encoding="utf-8")
        self.__data = load(self.__fn)

    def __enter__(self):
        return self.__data

    def __exit__(self, tipe, value, traceback):
        self.__fn.close()

