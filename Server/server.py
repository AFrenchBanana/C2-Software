#!/usr/bin/python3

"""
initial file that starts the socket and multi handler
"""

from Modules.multi_handler import MultiHandler
import readline
import colorama


if __name__ == '__main__':
    try:
        multi_handler = MultiHandler() # starts the multi handler socket
        multi_handler.create_certificate() # checks if certificates are available
        multi_handler.startsocket() # starts the socket
        print(colorama.Fore.CYAN + """
    d8888   .d8888b.  888888888     8888888 8888888b. 88888888888 
   d8P888  d88P  Y88b 888             888   888   Y88b    888     
  d8P 888  888    888 888             888   888    888    888     
 d8P  888  888    888 8888888b.       888   888   d88P    888     
d88   888  888    888      "Y88b      888   8888888P"     888     
8888888888 888    888        888      888   888 T88b      888     
      888  Y88b  d88P Y88b  d88P      888   888  T88b     888     
      888   "Y8888P"   "Y8888P"     8888888 888   T88b    888
""")
        print(colorama.Back.GREEN + "Type Help for available commands")
        multi_handler.multi_handler() # starst the milti handler
    except KeyboardInterrupt: # handles keyboard interpt
        print("\n use exit next time")