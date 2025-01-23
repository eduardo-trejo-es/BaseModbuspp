Using only TCP sockets to comply with the Modbus protocol. Here's a basic implementation in C++.

The Modbus TCP/IP protocol is based on a Modbus Application Protocol (MBAP) header followed by Modbus protocol data.

Modbus TCP/IP library code without external dependencies

Explanations
TCP connection: The client creates a TCP socket and connects to the specified IP address and port of the Modbus server.
Request construction: The message includes the MBAP header and Modbus protocol data.
Sending and receiving data: Requests are sent via send(), and responses are received with recv().
Response analysis: The response is validated to ensure that it conforms to the Modbus protocol.


Open the project in Vs code and tap Ctrl+Shift+B to build up, the all config to build up is done on task.json file 
