#include <time.h>
#include <etkxx.h>
#define BUF_SIZE	1500
#define DEFAULT_PORT	5000


static void server()
{
	ENetEndpoint endpoint(SOCK_STREAM);

	if(endpoint.InitCheck() != E_OK) ETK_ERROR("Failed to create endpoint: %s.", endpoint.ErrorStr());
	if(endpoint.Bind(DEFAULT_PORT) != E_OK) ETK_ERROR("Bind(%d) failed: %s.", DEFAULT_PORT, endpoint.ErrorStr());
	if(endpoint.Listen() != E_OK) ETK_ERROR("Listen() failed: %s.", endpoint.ErrorStr());

	while(true)
	{
		ETK_OUTPUT("Waiting for client ...\n");

		ENetEndpoint *connection = endpoint.Accept(500);
		if(connection == NULL)
		{
			ETK_WARNING("Accept() == NULL: %s.", endpoint.ErrorStr());
			continue;
		}

		struct in_addr addr;
		euint16 port;
		connection->RemoteAddr().GetAddr(addr, &port);

		euint32 ip = E_BENDIAN_TO_HOST_INT32(addr.s_addr);
		ETK_OUTPUT("connection from %d.%d.%d.%d, port: %d\n",
			   ip >> 24, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff, port);

		EString buf;
		time_t curTime = time(NULL);
		buf << ctime(&curTime) << "\r\n";
		connection->Send(buf.String(), buf.Length());

		delete connection;
	}
}


static void client(const char *address)
{
	ENetEndpoint endpoint(SOCK_STREAM);

	if(endpoint.InitCheck() != E_OK) ETK_ERROR("Failed to create endpoint: %s.", endpoint.ErrorStr());
	if(endpoint.Connect(address, DEFAULT_PORT) != E_OK) ETK_ERROR("Connect() failed: %s.", endpoint.ErrorStr());
	endpoint.SetTimeout(E_INFINITE_TIMEOUT);

	char buf[BUF_SIZE];
	size_t pos = 0;

	bzero(buf, BUF_SIZE);

	while(pos < BUF_SIZE - 1)
	{
		ETK_OUTPUT("Receiving ...\n");

		eint32 bytes = endpoint.Receive(buf + pos, BUF_SIZE - pos - 1);
		if(bytes < 0) break;

		pos += bytes;
		if(pos < 3) continue;

		if(buf[pos - 1] == '\n' && buf[pos - 2] == '\r') break;
	}

	ENetDebug::Enable(true);
	ENetDebug::Dump(buf, pos, "Received");

	ETK_OUTPUT("Received: %s", buf);
}


int main(int argc, char **argv)
{
	EPath aPath(argv[0]);

	ETK_OUTPUT("usage: %s [IP Address]\n", aPath.Leaf());

	if(argc == 1) server();
	else client(argv[1]);

	return 0;
}


