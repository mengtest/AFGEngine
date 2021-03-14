#ifndef NETPLAY_H_INCLUDED
#define NETPLAY_H_INCLUDED

#include <chrono>
#include <deque>
#include <mutex>
#include <asio.hpp>
#include "input.h"

bool Host(std::string portStr);
bool Join(std::string addressStr, std::string portStr);

class Pinged
{
public:
	int inputDelay;
	asio::ip::udp::endpoint remote;

private:
	asio::io_service *ios;
	asio::io_service timerIos;
	asio::ip::udp::socket *socket;

	asio::steady_timer timeOutTimer;
	bool expired;

	std::unique_lock<std::mutex> socketLock;
	std::mutex socketMutex;

public:
	Pinged(asio::io_service *_ios, asio::ip::udp::endpoint local);

private:
	void BlockingReceiveSend();
	void ReceiveMsg(int &receivedN, bool &success, const asio::error_code& error, std::size_t bytes);
	void TimedOut(const asio::error_code& error);

};

class Pinger
{
public:
	int inputDelay;

private:
	asio::io_service *ios_p;
	asio::steady_timer timedOut;
	bool failed;
	bool isTimedOut;
	asio::ip::udp::socket socket;

	int receivedN;
public:
	Pinger(asio::io_service &ios, asio::ip::udp::endpoint connectPoint, int tries);

private:
	void ReceiveReturnMsg(asio::error_code* ec, bool &success, const asio::error_code& error, std::size_t /*bytes_transferred*/);
	void TimerExpired(const asio::error_code& error);

};

struct Packet
{
	uint8_t inputs[8];
	int32_t tickNumber = -1;
};

class InputGetter
{
public:
	bool isReady;
	bool keepRunning;

	Packet lastPacket;

private:
	bool host;
	asio::io_service ios;
	asio::ip::udp::socket socket;

public:
	InputGetter(bool host);

	void Run(int *currentTick);
private:
	void BlockingGetLoop(int *currentTick);

};

class InputSender
{
private:
	asio::io_service ios;
	asio::ip::udp::socket socket;

public:
	InputSender();

	void Send(input_deque *inputList, const int currentTick);
};

#endif // NETPLAY_H_INCLUDED
