import socket
import struct
import threading
import ssl
from scapy.all import sniff, wrpcap
from Modules.content_handler import TomlFiles
from Modules.global_objects import connectionaddress
from ServerDatabase.database import DatabaseClass


class PacketSniffer:
    """Packet Sniffer function, can intercept packets on a raw socket"""

    def __init__(self):
        with TomlFiles("config.toml") as f:  # loads the config files
            self.config = f
        self.database = DatabaseClass() # loads database class
        # Initialize empty lists for storing connection details
        self.snifferdetails = []
        self.snifferaddress = []

    def start_raw_socket(self):
        """Starts a raw socket wrapped in SSL"""
        # Set up SSL context
        context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        context.load_cert_chain(certfile=self.config['packetsniffer']['TLSCertificate'], keyfile=self.config['packetsniffer']['TLSkey'])
        
        # Create and bind SSL socket
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((self.config['packetsniffer']['listenaddress'], self.config['packetsniffer']['port']))
        self.server_socket.listen()

        # Start listener thread
        listener_thread = threading.Thread(target=self.accept_raw_connection)
        listener_thread.start()

    def accept_raw_connection(self):
        """Listens for connections and handles them"""
        i = 0
        while True:
            conn, r_address = self.server_socket.accept()
            self.snifferdetails.append(conn)
            self.snifferaddress.append(r_address)
            shark_thread = threading.Thread(target=self.listener, args=(i,))
            shark_thread.start()
            i += 1

    def listener(self, i):
        """Listens and processes packets"""
        # Check if sudo is enabled on the client
        if self.config['packetsniffer']['debugPrint']:
            while True:
                try:
                    data = self.snifferdetails[i].recv(4096)  # receive packets

                    # unpacks mac addresses and ether type
                    ethernet_header = data[:14]  # grabs the ethernet header
                    ethernet_header_tuple = struct.unpack("!6s6sH", ethernet_header)  # decodes the ethernet header
                    source_mac = ":".join("{:02x}".format(x) for x in ethernet_header_tuple[0])  # grabs the source mac
                    destination_mac = ":".join("{:02x}".format(x) for x in ethernet_header_tuple[1])  # grabs the dest mac
                    ethertype = ethernet_header_tuple[2]  # ether type frame

                    if ethertype == 0x0806:  # arp header
                        arp_header_format = '!HHBBH6s4s6s4s'
                        arp_header = struct.unpack(arp_header_format, data[14:42])
                        hardware_type = arp_header[0]
                        protocol_type = arp_header[1]
                        hardware_size = arp_header[2]
                        protocol_size = arp_header[3]
                        opcode = arp_header[4]
                        sender_mac = ":".join("{:02x}".format(x) for x in arp_header[5])
                        sender_ip = '.'.join(map(str, arp_header[6]))
                        target_mac = ":".join("{:02x}".format(x) for x in arp_header[7])
                        target_ip = '.'.join(map(str, arp_header[8]))
                        if opcode == 1:
                            self.database.insert_entry("Sniffer", "ARP Whois", source_mac, destination_mac, sender_ip, f"WhoHas {target_ip}")  
                        elif opcode == 2:
                            self.database.insert_entry("Sniffer", "ARP Reply", source_mac, destination_mac, sender_ip, f"is at {sender_mac}")  

                    elif ethertype in (0x0800, 2048):  # IPv4
                        ip4_header = data[14:34]  # gets the ip header
                        ip4_header_tuple = struct.unpack("!BBHHHBBH4s4s", ip4_header)  # decodes the IP header
                        ip_version = ip4_header_tuple[0] >> 4  # ip version
                        ip_header_length = (ip4_header_tuple[0] & 0x0F) * 4  # header length
                        ttl = ip4_header_tuple[5]
                        protocol = ip4_header_tuple[6]
                        source_ip = socket.inet_ntoa(ip4_header_tuple[8])  # src ip
                        dest_ip = socket.inet_ntoa(ip4_header_tuple[9])  # dest ip

                        if protocol == 6:  # TCP
                            tcp_header = data[34:54]
                            tcp_header_tuple = struct.unpack('!HHLLBBHHH', tcp_header)
                            source_port = tcp_header_tuple[0]
                            dest_port = tcp_header_tuple[1]
                            sequence_num = tcp_header_tuple[2]
                            ack_num = tcp_header_tuple[3]
                            tcp_offset = (tcp_header_tuple[4] >> 4) * 4
                            tcp_flags = tcp_header_tuple[5]
                            window_size = tcp_header_tuple[6]
                            tcp_checksum = tcp_header_tuple[7]
                            urg_pointer = tcp_header_tuple[8]
                            self.database.insert_entry("Sniffer", "TCP", source_mac, destination_mac, source_ip, dest_ip)  

                        elif protocol == 17:  # UDP
                            udp_header = data[34:42]
                            udp_header_tuple = struct.unpack('!HHHH', udp_header)
                            source_port = udp_header_tuple[0]
                            dest_port = udp_header_tuple[1]
                            udp_length = udp_header_tuple[2]
                            udp_checksum = udp_header_tuple[3]
                            self.database.insert_entry("Sniffer", "UDP", source_mac, destination_mac, source_ip, dest_ip)  
                        

                    elif ethertype in (0x86DD, 34525):  # IPv6
                        ip6_header = data[14:54]
                        ip6_header_tuple = struct.unpack('!IHBB16s16s', ip6_header)
                        ip_version = (ip6_header_tuple[0] >> 28) & 0xF
                        traffic_class = (ip6_header_tuple[0] >> 20) & 0xFF
                        flow_label = ip6_header_tuple[0] & 0xFFFFF
                        payload_length = ip6_header_tuple[1]
                        next_header = ip6_header_tuple[2]
                        hop_limit = ip6_header_tuple[3]
                        source_ip = socket.inet_ntop(socket.AF_INET6, ip6_header_tuple[4])
                        dest_ip = socket.inet_ntop(socket.AF_INET6, ip6_header_tuple[5])
                        self.database.insert_entry("Sniffer", "IPv6", source_mac, destination_mac, source_ip, dest_ip)  

                except:
                    pass
        else:
            filename = "captured_packets.pcap"
            with open(filename, 'wb') as pcap_file:
                while True:
                    data = self.snifferdetails[i].recv(4096)  # receive packets
                    wrpcap("captured_packets.pcap", data)  # Write each packet to pcap



