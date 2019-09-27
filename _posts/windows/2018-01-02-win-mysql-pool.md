---
layout: post
title: mysql数据库连接池(win32版)
tags:
- windows
categories: windows
description: mysql数据库连接池(win32版)
---


本文是自己工作中写的一个WIN32版本MySQL数据库连接池。因网络上Win32版本MySQL连接池较少，这里开源，同时也方便自己后续使用。

* 操作系统: 64位win7

* 编译器： vs2013



<!-- more -->



## 1. 头文件PKV10_Mysql.h
{% highlight string %}
#ifndef _PKV10_MYSQL_H_INCLUDED_
#define _PKV10_MYSQL_H_INCLUDED_

#include "stdafx.h"
#include <string>
#include <set>
#include <list>
#include "mysql.h"



#define MYSQL_STATUS_UNCONNECTED   0x0
#define MYSQL_STATUS_CONNECTED     0x1

class MySQLConnectionPool;


class MySQLConnection{
public:
	MySQLConnection(MySQLConnectionPool *connection_pool){
		conn = NULL;
		conn_status = MYSQL_STATUS_UNCONNECTED;
		this->pConnectionPool = connection_pool;
		ownerQueue = NULL;

		lastUseTick = 0x0;

		lastFixTick = 0x0;
		failFixCnt = 0x0;
		nextFixPeriod = 0x0;
	}

	~MySQLConnection(){
		if (conn){
			mysql_close(conn);
		}
	}

	//先决条件： Init()成功
	bool Connect();


	bool Init(){
		if ((conn = mysql_init(NULL)) == NULL)
			return false;

		return true;
	}

	

	bool Ping(){
		if (mysql_ping(conn)){
			this->conn_status = MYSQL_STATUS_UNCONNECTED;
			return false;
		}
		lastUseTick = ::GetTickCount();    //Refresh the lastUseTick
		return true;
	}

	void MarkUnconnected(){
		this->conn_status = MYSQL_STATUS_UNCONNECTED;
	}

	bool IsGoodConnection(){
		return (conn_status == MYSQL_STATUS_CONNECTED);
	}

public:
	MYSQL *conn;  

private:
	int conn_status;        //连接状态   0----unconnect  1--connected
	MySQLConnectionPool *pConnectionPool;
	void *ownerQueue;
	DWORD lastUseTick;       //本连接最近一次被使用的时间（如果连接长时间不使用，需要使用ping来保活，只会对good_connections中的连接进行保活)

	DWORD lastFixTick;        //本连接在上一次进行修复时的时间戳
	DWORD failFixCnt;         //修复失败的次数
	DWORD nextFixPeriod;      //下一次修复的时间间隔

	friend class MySQLConnectionPool;
};

#define CONNECTION_PING_PERIOD           300000           //every 5min, we will try ping the idle connections    
class MySQLConnectionPool{
private:
	MySQLConnectionPool(){
		databaseURL = "";
		databasePort = 0x0;
		usrName = "";
		passwd = "";
		schema = "";
		max_conn = 0x0;
		
	
		::InitializeCriticalSection(&connCs);
		::InitializeConditionVariable(&goodNotEmpty);
		::InitializeConditionVariable(&tmpBadBufNotEmpty);

		bStop = false;
		poolThread = NULL;
	}

	~MySQLConnectionPool(); 
	

public:
	bool Init(char databaseURL[], int databasePort, char usrName[], char passwd[], char schema[], int max_conn);

	/*
	 * 说明： 本函数通常用于内部线程使用，外部不应该进行调用。
	 * 
	 * 由于MySQLConnection::Connect()是一个阻塞函数，因此InitialConnect()通常会放在线程中来进行初次连接，以防止程序被阻塞
	*/
	void InitialConnect();

	//说明： 用于等待当前已经有处于"成功"状态的空闲连接
	bool WaitGoodNotEmpty(DWORD dwMilliseconds);

	//说明：此处我们另外提供IsAllBad()函数来判断是否有“好”的连接，因为“好”的连接有可能被Borrow()出去了，因此我们并不能通过
	// 直接查看good_connections是否为空来判断（注： 非线程安全，不能严格判断当前所有连接都处于bad状态，通常我们只会用此函数来
	// 进行一些非严格的健康检查与程序恢复)
	bool IsAllBad();

	MySQLConnection *Borrow();

	MySQLConnection *Borrow(DWORD dwMilliseconds);

	void Recycle(MySQLConnection *conn);


	/*
	 * 说明：非线程安全函数，当前仅被MySQLConnectionPool内部线程使用 
	*/
	void WaitNewBadConnections_unsafe(DWORD dwMilliseconds);

	/*
	* 说明： 返回bad_connections中下一次最小的修复时间间隔
	* (注： 本函数为非线程安全函数，当前仅被MySQLConnectionPool的内部线程使用)
	*/
	DWORD FixBadConnections_unsafe();
	
	

	DWORD PingIdle();

	const inline std::string &GetDatabaseURL(){
		return databaseURL;
	}
	int inline GetDatabasePort(){
		return databasePort;
	}

	const inline std::string &GetUsrName(){
		return usrName;
	}

	const inline std::string &GetPassword(){
		return passwd;
	}

	const inline std::string &GetSchema(){
		return schema;
	}

	int inline GetMaxConn(){
		return max_conn;
	}

	static MySQLConnectionPool *GetInstance();




private:
	std::string databaseURL;
	int databasePort;
	std::string usrName;
	std::string passwd;
	std::string schema;
	int max_conn;
	


	std::set<MySQLConnection *> all_connections;
	std::list<MySQLConnection *> good_connections;    //采用list，可以让每一条连接以round robin方式进行获取
	/*
	 * 说明： tmpbad_buf用于临时保存bad connections。由于MySQLConnection::Connect()函数是一个阻塞操作，因此我们真正的重连
	 * 会放到下面的unsafe_bad_connections中来进行。如若不采用此临时缓冲，则在重连过程中可能会长期占用connCs这一互斥锁，从而
	 * 导致不能快速的获取到good_connections中的空闲连接
	 */
	std::set<MySQLConnection *> tmpbad_buf;          
	CRITICAL_SECTION connCs;                         //用于保护good_connections以及tmpbad_buf
	CONDITION_VARIABLE goodNotEmpty;
	CONDITION_VARIABLE tmpBadBufNotEmpty;            

	HANDLE poolThread;
	std::set<MySQLConnection *> unsafe_bad_connections;      //unsafe_bad_connections为非线程安全队列，当前仅会被poolThread内部使用

	bool bStop;
};




#endif
{% endhighlight %}


## 2. 源文件PKV10_Mysql.cpp
{% highlight string %}
#include "stdafx.h"
#include "log4z.h"
#include "PKV10_Mysql.h"


static DWORD WINAPI PoolThreadProc(LPVOID lpParameter);


//先决条件： Init()成功
bool MySQLConnection::Connect(){
	const std::string & databaseURL = pConnectionPool->GetDatabaseURL();
	const std::string & usrName = pConnectionPool->GetUsrName();
	const std::string & password = pConnectionPool->GetPassword();
	const std::string & schema = pConnectionPool->GetSchema();
	int databasePort = pConnectionPool->GetDatabasePort();

	if (mysql_real_connect(conn, databaseURL.c_str(), usrName.c_str(), password.c_str(), schema.c_str(), databasePort, NULL, 0x0) == NULL){
		LOGFMTE("connect %s:%d(%s) failure: %s", databaseURL.c_str(), databasePort, schema.c_str(), mysql_error(conn));
		
		this->conn_status = MYSQL_STATUS_UNCONNECTED;
		return false;
	}

	this->conn_status = MYSQL_STATUS_CONNECTED;
	return true;
}


MySQLConnectionPool::~MySQLConnectionPool(){
	std::set<MySQLConnection *>::iterator it;

	::WakeConditionVariable(&goodNotEmpty);
	::WakeConditionVariable(&tmpBadBufNotEmpty);

	good_connections.clear();
	tmpbad_buf.clear();
	unsafe_bad_connections.clear();

	for (it = all_connections.begin(); it != all_connections.end(); ){
		MySQLConnection *conn = *it;
		delete conn;
		all_connections.erase(it++);
	}

	DeleteCriticalSection(&connCs);
}

bool MySQLConnectionPool::Init(char databaseURL[], int databasePort, char usrName[], char passwd[], char schema[], int max_conn){
	this->databaseURL = std::string(databaseURL);
	this->databasePort = databasePort;
	this->usrName = std::string(usrName);
	this->passwd = std::string(passwd);
	this->schema = std::string(schema);

	this->max_conn = max_conn;
	if (this->max_conn <= 0)
		this->max_conn = 5;
	else if (max_conn > 50)
		this->max_conn = 50;

	for (int i = 0x0; i < this->max_conn; i++){
		MySQLConnection *conn = new MySQLConnection(this);
		if (conn->Init() == false){
			delete conn;
			return false;
		}
		all_connections.insert(conn);
	}


	poolThread = ::CreateThread(NULL, 0x0, PoolThreadProc, this, 0x0, NULL);
	if (poolThread == NULL){
		return false;
	}

	return true;
}

/*
* 说明： 本函数通常用于内部线程使用，外部不应该进行调用。
*
* 由于MySQLConnection::Connect()是一个阻塞函数，因此InitialConnect()通常会放在线程中来进行初次连接，以防止程序被阻塞
*/
void MySQLConnectionPool::InitialConnect(){
	std::set<MySQLConnection *>::iterator it;
	DWORD currentTick;

	EnterCriticalSection(&connCs);

	for (it = all_connections.begin(); it != all_connections.end(); it++){
		MySQLConnection *pConnection = *it;
		currentTick = ::GetTickCount();

		if (pConnection->Connect())
		{
			pConnection->lastUseTick = currentTick;
			pConnection->ownerQueue = &good_connections;
			good_connections.push_back(pConnection);
		}
		else{
			pConnection->ownerQueue = &unsafe_bad_connections;
			pConnection->lastFixTick = currentTick;
			pConnection->failFixCnt = 0;
			pConnection->nextFixPeriod = 0;
			unsafe_bad_connections.insert(pConnection);
		}
	}

	if (good_connections.empty() == false){
		::WakeConditionVariable(&goodNotEmpty);
	}

	LeaveCriticalSection(&connCs);
}

//说明： 用于等待当前已经有处于"成功"状态的空闲连接
bool MySQLConnectionPool::WaitGoodNotEmpty(DWORD dwMilliseconds){
	bool ret;

	EnterCriticalSection(&connCs);
	if (good_connections.empty())
	{
		::SleepConditionVariableCS(&goodNotEmpty, &connCs, dwMilliseconds);
	}

	if (!good_connections.empty())
		ret = true;
	else
		ret = false;
	LeaveCriticalSection(&connCs);

	return ret;
}


//说明：此处我们另外提供IsAllBad()函数来判断是否有“好”的连接，因为“好”的连接有可能被Borrow()出去了，因此我们并不能通过
// 直接查看good_connections是否为空来判断（注： 非线程安全，不能严格判断当前所有连接都处于bad状态，通常我们只会用此函数来
// 进行一些非严格的健康检查与程序恢复)
bool MySQLConnectionPool::IsAllBad(){
	int badCnt = unsafe_bad_connections.size();

	return badCnt == max_conn;
}


MySQLConnection * MySQLConnectionPool::Borrow(){
	MySQLConnection *mysqlConn = NULL;
	EnterCriticalSection(&connCs);

	while (good_connections.empty()){
		SleepConditionVariableCS(&goodNotEmpty, &connCs, INFINITE);
	}

	mysqlConn = good_connections.front();
	mysqlConn->ownerQueue = NULL;
	good_connections.pop_front();

	LeaveCriticalSection(&connCs);
	return mysqlConn;
}

MySQLConnection *MySQLConnectionPool::Borrow(DWORD dwMilliseconds){
	MySQLConnection *mysqlConn = NULL;
	EnterCriticalSection(&connCs);

	if (good_connections.empty()){
		SleepConditionVariableCS(&goodNotEmpty, &connCs, dwMilliseconds);
	}

	if (!good_connections.empty()){
		mysqlConn = good_connections.front();
		good_connections.pop_front();
		mysqlConn->ownerQueue = NULL;
	}

	LeaveCriticalSection(&connCs);
	return mysqlConn;
}

    
void MySQLConnectionPool::Recycle(MySQLConnection *conn){
	std::set<MySQLConnection *>::iterator it;
	if (!conn)
		return;
	
	EnterCriticalSection(&connCs);
	
	it = all_connections.find(conn);
	if (it == all_connections.end())
	{
		LeaveCriticalSection(&connCs);
		return;
	}

	if (conn->ownerQueue != NULL){
		LeaveCriticalSection(&connCs);
		return;
	}

	if (conn->IsGoodConnection()){
		if (good_connections.empty()){
			good_connections.push_back(conn);
			conn->ownerQueue = &good_connections;
			::WakeConditionVariable(&goodNotEmpty);
		}
		else{
			good_connections.push_back(conn);
			conn->ownerQueue = &good_connections;
		}

		conn->lastUseTick = ::GetTickCount();     //Refresh the lastUseTick
	}
	else{
		if (tmpbad_buf.empty()){
			tmpbad_buf.insert(conn);
			::WakeConditionVariable(&tmpBadBufNotEmpty);
		}
		else{
			tmpbad_buf.insert(conn);
		}
		conn->ownerQueue = &tmpbad_buf;
		conn->failFixCnt = 0x0;
		conn->nextFixPeriod = 0x0;
		conn->lastFixTick = 0x0;
	}

	LeaveCriticalSection(&connCs);
	return;
}


//说明： 本函数返回的是bad_connections中当前最小的修复时间间隔，我们会以该时间间隔来启动下一次修复。这可能使得
//对于有一些连接的修复时间被推迟（这里我们采用指数退避的方式进行重连，这个推迟时间在实际环境中应该不会有太大的
//问题，并且可以避免出现在短时间内因断联而导致的太过频繁的重连发生）
DWORD MySQLConnectionPool::FixBadConnections_unsafe(){
	std::set<MySQLConnection *>::iterator it;
	DWORD nextFixTimeout = INFINITE;
	DWORD currentTick = ::GetTickCount();
	DWORD timeElapse = 0x0;
	

	badcnt = 0;
	for (it = unsafe_bad_connections.begin(); it != unsafe_bad_connections.end();){
		MySQLConnection *pConnection = *it;

		if (currentTick < pConnection->lastFixTick){
			//tickcnt has overflow
			timeElapse = 0xFFFFFFFF - pConnection->lastFixTick + currentTick;
		}
		else{
			timeElapse = currentTick - pConnection->lastFixTick;
		}

		if (pConnection->nextFixPeriod <= timeElapse){
			//should reconnect here
			pConnection->lastFixTick = currentTick;

			if (pConnection->Connect()){
				unsafe_bad_connections.erase(it++);

				EnterCriticalSection(&connCs);
				if (good_connections.empty()){
					good_connections.push_back(pConnection);
					::WakeConditionVariable(&goodNotEmpty);
				}
				else{
					good_connections.push_back(pConnection);
				}
				pConnection->ownerQueue = &good_connections;
				pConnection->conn_status = MYSQL_STATUS_CONNECTED;
				pConnection->lastUseTick = currentTick;           //Refresh the lastUseTick

				pConnection->lastFixTick = 0;
				pConnection->failFixCnt = 0;
				pConnection->nextFixPeriod = 0x0;
				LeaveCriticalSection(&connCs);
			}
			else{
				//才用指数退避的方式进行重连
				pConnection->failFixCnt++;
				if (pConnection->nextFixPeriod == 0){
					pConnection->nextFixPeriod = 200;               
				}
				else{
					pConnection->nextFixPeriod *= 2;
				}

				if (pConnection->nextFixPeriod >= 40000)       //40s
					pConnection->nextFixPeriod = 40000;
				
				if (pConnection->nextFixPeriod < nextFixTimeout){
					nextFixTimeout = pConnection->nextFixPeriod;
				}

				it++;        
			}

		}
		else{
			if (pConnection->nextFixPeriod < nextFixTimeout){
				nextFixTimeout = pConnection->nextFixPeriod;
			}
			it++;
		}
		
	}

	return nextFixTimeout;
}

void MySQLConnectionPool::WaitNewBadConnections_unsafe(DWORD dwMilliseconds){
	std::set<MySQLConnection *>::iterator it;

	EnterCriticalSection(&connCs);
	if (tmpbad_buf.empty()){
		::SleepConditionVariableCS(&tmpBadBufNotEmpty, &connCs, dwMilliseconds);
	}

	/*
	 * 说明：WaitNewBadConnections_unsafe与FixBadConnections_unsafe()工作于同一内部线程，此处将tmpbad_buf中的连接添加到
	 * unsafe_bad_connections队列中是不会出问题的
	*/
	for (it = tmpbad_buf.begin(); it != tmpbad_buf.end(); it++){
		MySQLConnection *pConnection = *it;
		tmpbad_buf.erase(it++);

		pConnection->ownerQueue = &unsafe_bad_connections;
		unsafe_bad_connections.insert(pConnection);
	}
	LeaveCriticalSection(&connCs);
}

DWORD MySQLConnectionPool::PingIdle(){
	std::list<MySQLConnection *>::iterator it;
	bool ret;

	DWORD nextPingPeriod = CONNECTION_PING_PERIOD;
	DWORD currentTick = ::GetTickCount();
	DWORD timeElapse;


	EnterCriticalSection(&connCs);
	for (it = good_connections.begin(); it != good_connections.end(); ){
		MySQLConnection *pConnection = (*it);

		if (currentTick < pConnection->lastUseTick){
			//tick overflow
			timeElapse = 0xFFFFFFFF - pConnection->lastUseTick + currentTick;
		}
		else{
			timeElapse = currentTick - pConnection->lastUseTick;
		}
		if (timeElapse < CONNECTION_PING_PERIOD){
			nextPingPeriod = CONNECTION_PING_PERIOD - timeElapse;
			break;
		}
		ret = pConnection->Ping();
		if (ret == false){
			//find an bad connection
			good_connections.erase(it++);
			pConnection->failFixCnt = 0x0;
			pConnection->nextFixPeriod = 0;
			pConnection->ownerQueue = &tmpbad_buf;

			if (tmpbad_buf.empty()){
				tmpbad_buf.insert(pConnection);
				::WakeConditionVariable(&tmpBadBufNotEmpty);
			}
			else{
				tmpbad_buf.insert(pConnection);
			}
		}
		else{
			it++;
		}
	}

	LeaveCriticalSection(&connCs);

	return nextPingPeriod;
}

MySQLConnectionPool * MySQLConnectionPool::GetInstance(){
	static MySQLConnectionPool thePool;

	return &thePool;
}


   
DWORD WINAPI PoolThreadProc(LPVOID lpParameter){
	MySQLConnectionPool *thePool = (MySQLConnectionPool *)lpParameter;
	DWORD nextWaitTime = INFINITE;
	DWORD currentTick;
	DWORD lastPingTick = ::GetTickCount();
	DWORD nextPingPeriod = CONNECTION_PING_PERIOD;
	DWORD timeElapse;
	

	thePool->InitialConnect();
	nextWaitTime = thePool->FixBadConnections_unsafe();
	while (1){
		if (nextWaitTime > CONNECTION_PING_PERIOD){
			nextWaitTime = CONNECTION_PING_PERIOD;
		}
		else if (nextWaitTime < 100){
			nextWaitTime = 100;      //100ms
		}

		thePool->WaitNewBadConnections_unsafe(nextWaitTime);
		nextWaitTime = thePool->FixBadConnections_unsafe();

		//ping idle check
		currentTick = ::GetTickCount();
		if (currentTick < lastPingTick){
			timeElapse = INFINITE - lastPingTick + currentTick;
		}
		else{
			timeElapse = currentTick - lastPingTick;
		}
		if (timeElapse >= nextPingPeriod){
			nextPingPeriod = thePool->PingIdle();
			lastPingTick = currentTick;
		}
		
		if (nextWaitTime > nextPingPeriod)
			nextWaitTime = nextPingPeriod;
	}
	
}
{% endhighlight %}



<br />
<br />
**[参看]:**





<br />
<br />
<br />





