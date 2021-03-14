#include <cmath>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include <string>
#include <asio.hpp>

#include "netplay.h"
#include "main.h"
#include "input.h"

asio::ip::udp::endpoint connectedTo;
asio::ip::udp::endpoint remoteg;
asio::ip::udp::socket *gsocket;
asio::ip::udp::socket *gsocket22;
asio::io_service *gios = new asio::io_service;
unsigned short portHost = 0;

int NetplayArgs()
{
		netplay = true;
		bool success = false;
		if(argc == 3)
		{
			std::string address(argv[1]);
			std::string port(argv[2]);
			std::cout << "Joining " << address << " at " << port << "...\n";
			success = Join(address, port);
		}
		else if(argc == 2)
		{
			std::string port(argv[1]);
			std::cout << "Hosting at " << port << "...\nCTRL+C to close.\n\n";
			success = Host(port);
			host = true;
		}

		if(success)
		{
			std::cout << "Enter input delay (0-30): ";
            std::cin >> inputDelay;
            if(!std::cin.good())
			{
				std::cout << "Invalid input.\n";
				std::cin.get();
				return 0;
			}
		}
		else
		{
			std::cout << "\nUsage: \tHosting: <port>\n\t\tJoining: <adress> <port>\n";
			return 0;
		}
}

bool Host(std::string portStr)
{
	unsigned short port = 0;
	try
	{
		port = std::stoi(portStr);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << " exception: Invalid port number: " << port << "\n";
		return false;
	}

	using asio::ip::udp;

	udp::endpoint local(udp::v4(), port);
	Pinged pingReceiver(gios, local);

	if(pingReceiver.inputDelay < 0)
	{
		std::cout << "\nCouldn't connect.\n";
		return false;
	}

	portHost = port;
	return true;
}

bool Join(std::string addressStr, std::string portStr)
{
	unsigned short port = 0;
	asio::ip::address_v4 address;
	try
	{
		port = std::stoi(portStr);
		address = asio::ip::address_v4::from_string(addressStr);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << " exception at Join()\n";
		return false;
	}

	try
	{
		using asio::ip::udp;

		//asio::io_service ios;

		udp::endpoint connectPoint(address, port);
		Pinger pingTheHost(*gios, connectPoint, 5);

		if(pingTheHost.inputDelay < 0)
		{
			std::cout << "\nCouldn't connect.\n";
			return false;
		}
		connectedTo = connectPoint;

	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		std::cin.get();
		return false;
	}

	return true;
}


Pinged::Pinged(asio::io_service *_ios, asio::ip::udp::endpoint local) :
	inputDelay(-1),
	ios(_ios), timeOutTimer(timerIos, std::chrono::duration<int>::max()), expired(false), socketLock(socketMutex, std::defer_lock)
{
	gsocket = new asio::ip::udp::socket(*_ios, local);
	std::thread t1(Pinged::BlockingReceiveSend, this);

	while(!expired)
	{
		timerIos.run_one();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		timerIos.reset();
	}

	t1.join();
	//gsocket = socket;
	remoteg = remote;
}

void Pinged::BlockingReceiveSend()
{
	int tries = 0;
	int receivedN = -1; //-1 instead of 0 since first connection doesn't count because it can't measure the connection time.
	bool success = false;

	const auto expireFunc = std::bind(Pinged::TimedOut, this, std::placeholders::_1);
	const auto recvFunc = std::bind(Pinged::ReceiveMsg, this, std::ref(receivedN), std::ref(success), std::placeholders::_1, std::placeholders::_2);
	using asio::ip::udp;

	asio::mutable_buffers_1 firstRecvBuf(&tries, sizeof(int));

	int requestN = 0;
	asio::mutable_buffers_1 recvBuf(&requestN, sizeof(int));
	asio::mutable_buffers_1 sendBuf(&requestN, sizeof(int));
	std::cout << "Waiting for connection...\n";

	std::chrono::high_resolution_clock::time_point startT;
	std::chrono::high_resolution_clock::time_point endT;
	std::chrono::high_resolution_clock::time_point difT;

	try
	{
		gsocket->receive_from(firstRecvBuf, remote);
		gsocket->connect(remote);
		gsocket->send(sendBuf);

		startT = std::chrono::high_resolution_clock::now();

		timeOutTimer.expires_from_now(std::chrono::seconds(3));
		timeOutTimer.async_wait(expireFunc);
		std::cout << remote.address().to_string() << " has connected.\nClient is sending " << tries << " datagrams.\n";


		++receivedN;
		for(int i = 0; i < tries; ++i)
		{
			//std::cout << i << "---" << tries-1 << "\n";
			socketLock.lock(); //We don't want it to connect after being disconnected in another thread.
			if(expired)
				return;
			ios->reset();
			gsocket->connect(remote);
			socketLock.unlock();


			gsocket->async_receive(recvBuf, recvFunc);
			ios->run();


			//Calculate roundtrip time after sucessfully receiving.
			if(success)
			{
				endT = std::chrono::high_resolution_clock::now();
				difT += endT - startT;
				success = false;
			}

			gsocket->send(sendBuf);

			startT = std::chrono::high_resolution_clock::now();

			timeOutTimer.expires_from_now(std::chrono::seconds(3));
			timeOutTimer.async_wait(expireFunc);
		}
		timeOutTimer.cancel();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	if(!expired)
	{
		if(receivedN == tries)
			std::cout << "Successful connection. Received all packets.\n";
		else
			std::cout << "Lost " << tries - receivedN << " packets.\n";

		long long tics = difT.time_since_epoch().count() / (2*(receivedN));
		inputDelay = std::ceil(((tics/1000000000.f)+0.006)/SPF);
		std::cout << tics << " average per one way trip. That's about "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(difT.time_since_epoch()).count()/(2*(tries-1)) << "ms.\n"
			<< "Recommended input delay: " << inputDelay << "\n";
		expired = true;
	}
}

void Pinged::ReceiveMsg(int &receivedN, bool &success, const asio::error_code& error, std::size_t bytes)
{
	if(!error.value())
	{
		++receivedN;
		success = true;
	}
	return;
}

void Pinged::TimedOut(const asio::error_code& ec)
{
	if (ec != asio::error::operation_aborted) // Timer was not cancelled. It expired naturally, as it should be.
	{
		std::cout << "Timed out. Connection closed.\n";
		socketLock.lock();
		expired = true;

		ios->stop();
		gsocket->close();
		socketLock.unlock();
	}
	/*else
	{
		std::cout << "restarted timer.\n";
	}*/
}

Pinger::Pinger(asio::io_service &ios, asio::ip::udp::endpoint connectPoint, int tries) :
	inputDelay(-1),
	ios_p(&ios), timedOut(ios, std::chrono::duration<int>::max()), failed(false), isTimedOut(false), socket(ios, asio::ip::udp::v4()), receivedN(0)
{
	using asio::ip::udp;

	gsocket = new udp::socket(ios, asio::ip::udp::v4());

	std::chrono::high_resolution_clock::time_point startT;
	std::chrono::high_resolution_clock::time_point endT;
	std::chrono::high_resolution_clock::time_point difT;

	std::cout << "Connecting...\n";

	asio::mutable_buffers_1 sendBuf(&tries, sizeof(int));
	int receiveNum = 0;
	asio::mutable_buffers_1 recvBuf(&receiveNum, sizeof(int));

	std::cin.get();
	for(int i = 0; i < tries; ++i)
	{
		gsocket->connect(connectPoint);

		asio::error_code ec;
		gsocket->send(sendBuf, 0, ec);
		if(ec.value())
			std::cout << "Sending error: " << ec.message() << "\n";

		startT = std::chrono::high_resolution_clock::now();

		timedOut.expires_from_now(std::chrono::seconds(1));
		timedOut.async_wait(std::bind(Pinger::TimerExpired, this, std::placeholders::_1));

		bool success = false;
		ec = asio::error::would_block;
		gsocket->async_receive(recvBuf, std::bind(Pinger::ReceiveReturnMsg, this, &ec, std::ref(success), std::placeholders::_1, std::placeholders::_2));

		do
		{
			ios.poll_one();
		}while(ec == asio::error::would_block);//gives time to run the receive handle.

		if(success)
		{
			endT = std::chrono::high_resolution_clock::now();
			difT += endT - startT;
			success = false;
		}

		/*asio::steady_timer waitTimer(ios);
		waitTimer.expires_from_now(std::chrono::seconds(1));
		waitTimer.wait();
		std::cout << "...\n\n";*/

		isTimedOut = false;
		failed = false;
	}

	gsocket->send(sendBuf); //Extra send so both host and client get "tries" number of received packets. Otherwise client would get 1 more than the host.

	if(receivedN == tries)
		std::cout << "Successful connection. Received all packets.\n";
	else
		std::cout << "Lost " << tries - receivedN << " packets.\n";

	if(receivedN == 0) //connection failed completely.
		return;

	long long tics = difT.time_since_epoch().count() / (2*(receivedN));
	inputDelay = std::ceil(((tics/1000000000.f)+0.006)/SPF);
	std::cout << tics << " average per one way trip. That's about "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(difT.time_since_epoch()).count()/(2*(tries-1)) << "ms.\n"
		<< "Recommended input delay: " << inputDelay << "\n";
}

void Pinger::ReceiveReturnMsg(asio::error_code *ec, bool &success, const asio::error_code& error, std::size_t /*bytes_transferred*/)
{
	*ec = error;
	if(error.value() != 0)
	{
		std::cout << error.message() << " : " << error.value() << "\n";
		if(error != asio::error::operation_aborted) //Tell it didn't time out.
			failed = true;
	}
	else
	{
		++receivedN;
		success = true;
		timedOut.cancel();
	}
}

void Pinger::TimerExpired(const asio::error_code& error)
{
	if (error == asio::error::operation_aborted)
	{
		//timedOut.expires_from_now(std::chrono::seconds(0));
		return;
	}
	else if(failed) //Recv msg failed before time out.
	{
		std::cout << "Couldn't receive.\n";
		gsocket->close();
	}
	else
	{
		std::cout << "Timed out!\n";
		gsocket->close();
	}
	isTimedOut = true;
}

InputGetter::InputGetter(bool _host) :
	isReady(false), keepRunning(true), host(_host),
	socket(ios, asio::ip::udp::v4())
{
	using asio::ip::udp;
	/*if(host)
	{
		udp::endpoint local(udp::v4(), portHost);
		socket.bind(local);
	}*/
}

void InputGetter::Run(int *currentTick)
{
	std::thread t1(InputGetter::BlockingGetLoop, this, currentTick);
	t1.detach();
}

void InputGetter::BlockingGetLoop(int *currentTick)
{
	using asio::ip::udp;
	Packet tmpPacket;
	asio::mutable_buffers_1 recvBuf(&tmpPacket, sizeof(Packet));

	try
	{
		std::cout << gsocket->remote_endpoint().address().to_string() << ":" << gsocket->remote_endpoint().port() << " hosting shit!!\n";
		while(keepRunning)
		{
			//gsocket->connect(remoteg);
			gsocket->receive(recvBuf);
			std::cout << "got " << (int)tmpPacket.inputs[0] << "\n";
			//if(tmpPacket.tickNumber >= *currentTick)
				lastPacket = tmpPacket;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		std::cin.get();
		return false;
	}
}

InputSender::InputSender() :
	socket(ios, asio::ip::udp::v4())
{

}

void InputSender::Send(input_deque *inputList, const int currentTick)
{
    Packet packet;
    gsocket->connect(connectedTo);
    packet.inputs[0] = static_cast<uint8_t>(inputList->front());
    std::cout << "sending " << (int)packet.inputs[0] << "\n";
    packet.tickNumber = currentTick;
    gsocket->send(asio::const_buffers_1(&packet, sizeof(Packet)));
}
