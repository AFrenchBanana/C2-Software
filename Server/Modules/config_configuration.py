import readline
from typing import Tuple

from Modules.content_handler import TomlFiles
from Modules.global_objects import tab_compeletion

def config_menu() -> None: 
    while True:
        print("Config Menu")
        print("1. Show Config")
        print("2. Edit Config")
        print("3. Exit")
        readline.parse_and_bind("tab: complete")
        readline.set_completer(lambda text, state: tab_compeletion(text, state, ["1", "2", "3"]))
        inp = input("Enter Option: ")
        if inp == "1":
            show_config()
        if inp == "2":
            edit_config()
        if inp == "3":
            return



def show_config() -> None:
    """Shows the config TOML configuration"""
    config = ""
    with open("config.toml", "r") as f:
        file = f.readlines()
    for line in file:
        if line.startswith("[MultiHandlerCommands]"):
            break
        config += line
    print(config)
    

    
def edit_config() -> bool: 
    main_keys = ["server", "authentication", "packetsniffer", "exit"]
    server_keys = ["listenaddress", "port", "TLSCertificateDir", "TLSCertificate", "TLSkey", "GUI", "quiet_mode"]
    authentication_keys = ["keylength"]
    packetsniffer_keys = ["active", "listenaddress", "port", "TLSCertificate", "TLSKey", "debugPrint"]
    toml_file = TomlFiles('config.toml')
    with toml_file as config:
        while True:
            readline.parse_and_bind("tab: complete")
            readline.set_completer(lambda text, state: tab_compeletion(text, state, main_keys))
            key = input("Enter key: ")
            if key == "exit":
                return False
            if key not in main_keys:
                print("Not a valid key")
                pass
            if key == "server":
                readline.parse_and_bind("tab: cowmplete")
                readline.set_completer(lambda text, state: tab_compeletion(text, state, server_keys))
            elif key == "authentication":
                readline.parse_and_bind("tab: complete")
                readline.set_completer(lambda text, state: tab_compeletion(text, state, authentication_keys))
            elif key == "packetsniffer":
                readline.parse_and_bind("tab: complete")
                readline.set_completer(lambda text, state: tab_compeletion(text, state, packetsniffer_keys))
            subkey = input("Enter sub-key to change: ")
            if subkey not in config[key]:
                print("Invalid subkey")
                pass
            print("Current value: ", config[key][subkey])
            new_value = input("Enter new value: ")
            if isinstance(config[key][subkey], bool):
                if new_value.lower() not in ["true", "false"]:
                    print("Invalid value. Please enter true or false")
                else:
                    update = True
                    if new_value.lower() == "true":
                        new_value = True
                    else:
                        new_value = False
            elif isinstance(config[key][subkey], int):
                if not new_value.isdigit():
                    print("Invalid value. Please enter a number") 
                else:
                    update = True
                    new_value = int(new_value)                
            elif subkey == "listenaddress" and not new_value.replace(".", "").isdigit():
                print("Invalid value. Please enter a valid IP address")
            else:
                update = True
            if update:
                toml_file.update_config(key, subkey, new_value)
                print("Changes saved successfully!")
                print("Restart the server to apply changes.")
                return True
                