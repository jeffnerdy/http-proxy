#include "functions.h"

void parsePacket(std::string& packet, std::string& ip, unsigned short& port)
{
	std::string port_str;

	// get ip address
	int pos = packet.find('@');
	ip = packet.substr(0, pos);
	packet.erase(0, pos + 1);

	// get port number
	pos = packet.find('@');
	port_str = packet.substr(0, pos);
	port = static_cast<unsigned short>(stoi(port_str));
	packet.erase(0, pos + 1);
}