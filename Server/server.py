#!/usr/bin/python3

"""
initial file that starts the socket and multi handler
"""

from Modules.multi_handler import MultiHandler
import readline
import colorama
import sys  
from Modules.content_handler import TomlFiles


if __name__ == '__main__':
  with TomlFiles("config.toml") as file:
    config = file
  try:
      multi_handler = MultiHandler() # starts the multi handler socket
      multi_handler.create_certificate() # checks if certificates are available
      multi_handler.startsocket() # starts the socket
      print(colorama.Fore.CYAN + """
      CCCCCCCCCCCCC 222222222222222    
    CCC::::::::::::C2:::::::::::::::22  
  CC:::::::::::::::C2::::::222222:::::2 
C:::::CCCCCCCC::::C2222222     2:::::2 
C:::::C       CCCCCC            2:::::2 
C:::::C                          2:::::2 
C:::::C                       2222::::2  
C:::::C                  22222::::::22   
C:::::C                22::::::::222     
C:::::C               2:::::22222        
C:::::C              2:::::2             
C:::::C       CCCCCC2:::::2             
C:::::CCCCCCCC::::C2:::::2       222222
  CC:::::::::::::::C2::::::2222222:::::2
    CCC::::::::::::C2::::::::::::::::::2
      CCCCCCCCCCCCC22222222222222222222
""")
      print(colorama.Back.GREEN + "Type Help for available commands")
      multi_handler.multi_handler() # starst the milti handler
  except ValueError: # handles keyboard interpt
      print("\n use exit next time")
      try:
        sys.exit(app.exec_())
      finally:
        sys.exit()
