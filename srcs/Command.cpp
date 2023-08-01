#include "Command.hpp"

void	Command::runCommand(std::vector<std::string> token, Server *server, Client *client)
{
	if (token[1] == "PASS") {
		if (token.size() != 3)
			throw std::runtime_error("Invalid argument");
		pass(server, client, token[2]);
	}
	else if (token[1] == "NICK") {
		if (token.size() != 3)
			throw std::runtime_error("Invalid argument");
		nick(server, client, token[2]);
	}
	else if (token[1] == "USER") {
		if (token.size() != 6)
			throw std::runtime_error("Invalid argument");
		if (token[3] != "0" || token[4] != "*" || token[5][0] != ':')
			throw std::runtime_error("Invalid argument");
		user(server, client, token[2], token[5].substr(1));
	}
	else if (token[1] == "JOIN") {
		if (token.size() != 3 && token.size() != 4)
			throw std::runtime_error("Invalid argument");
		if (token[2][0] != '#')
			throw std::runtime_error("Invalid argument");
		join(server, client, token);
	}
	else if (token[1] == "PRIVMSG") {
		if (token.size() != 4)
			throw std::runtime_error("Invalid argument");
		privmsg(server, client, token[2], token[3]);
	}
	else if (token[1] == "KICK") {
		kick(server, client, token);
	}
	else if (token[1] == "INVITE") {
		if (token.size() != 4)
			throw std::runtime_error("Invalid argument");
		if (token[3][0] != '#')
			throw std::runtime_error("Invalid argument");
		invite(server, client, token[2], token[3].substr(1));
	}
	else if (token[1] == "TOPIC") {
		// TOPIC channel topic
		if (token.size() != 3 && token.size() != 4)
			throw std::runtime_error("Invalid argument");
		if (token[2][0] != '#')
			throw std::runtime_error("Invalid argument");
		topic(server, client, token);
	}
	else if (token[1] == "MODE") {
		if (token.size() != 4 && token.size() != 5)
			throw std::runtime_error("Invalid argument");
		mode(server, client, token);
	}
}

void	Command::pass(Server *server, Client *client, std::string password)
{	
	if (client->getAuthorized() >= 1)
		throw std::runtime_error("Already authorized");
	if (server->getPassword() == password)
		client->setAuthorized(1);
	else
		throw std::runtime_error("Wrong password");
}

void	Command::nick(Server *server, Client *client, std::string nickName)
{
	if (!client->getAuthorized())
		throw std::runtime_error("Not authorized");
	
	if (server->checkNickName(nickName))
		throw std::runtime_error("NickName already exist");
	
	client->setNickName(nickName);
}

void	Command::user(Server *server, Client *client, std::string userName, std::string realName)
{
	if (!client->getAuthorized())
		throw std::runtime_error("Not authorized");
	
	if (server->checkUserName(userName))
		throw std::runtime_error("UserName already exist");

	if (server->checkRealName(realName))
		throw std::runtime_error("RealName already exist");
	client->setUserName(userName);
	client->setRealName(realName);
}

void	Command::join(Server *server, Client *client, std::vector<std::string> token)
{
	if (!client->getAuthorized())
		throw std::runtime_error("Not authorized");
	
	std::string channelName = token[2].substr(1);
	std::string key = "";

	if (token.size() == 4)
		key = token[3];
	
	if (!server->checkChannelName(channelName))
	{
		server->addChannel(channelName);
		Channel *channel = server->getChannelByChannelName(channelName);
		channel->setKey(key);
		if (channel->getLimit() == -1 || channel->getLimit() > static_cast<int>(channel->getClientList().size()))
		{
			channel->addClient(client);
			client->addChannel(channelName, channel);
		}
		else
			throw std::runtime_error("Exceed user limit");
	}
	else
	{
		Channel *channel = server->getChannelByChannelName(channelName);
		if (channel == NULL)
			throw std::runtime_error("Channel doesn't exist");
		if (channel->getKey() == key)
		{
			if (channel->getLimit() == -1 || channel->getLimit() > static_cast<int>(channel->getClientList().size()))
			{
				channel->addClient(client);
				client->addChannel(channelName, channel);
			}
			else
				throw std::runtime_error("Exceed user limit");
		}
		else
			throw std::runtime_error("Wrong key");
	}
}

void	Command::privmsg(Server *server, Client *client, std::string target, std::string message)
{
	if (!client->getAuthorized())
		throw std::runtime_error("Not authorized");
	message = client->getNickName() + ": " + message + "\n";
	if (target[0] == '#')
	{
		target = target.substr(1);
		if (!server->checkChannelName(target))
			throw std::runtime_error("Channel doesn't exist");
		
		std::map<int, Client*> clientList = server->getChannelByChannelName(target)->_clientList;
		std::map<int, Client*>::iterator it = clientList.begin();
		while (it != clientList.end())
		{
			if (!send((*it).second->getFd(), message.c_str(), message.length(), 0))
				throw std::runtime_error("User not connected");
			it++;
		}
	}
	else
	{
		if (!server->checkNickName(target))
			throw std::runtime_error("NickName doesn't exist");
		if (!send(server->getFdByNickName(target), message.c_str(), message.length(), 0))
			throw std::runtime_error("User not connected");
	}
}

void	Command::kick(Server *server, Client *client, std::vector<std::string> token)
{
	if (token.size() != 4 && token.size() != 5)
		throw std::runtime_error("Invalid argument");
	if (token[2][0] != '#')
		throw std::runtime_error("Invalid argument");
	if (token.size() == 5)
	{
		if (token[4][0] != ':')
			throw std::runtime_error("Invalid argument");
	}
	
	if (!client->getAuthorized())
		throw std::runtime_error("Not authorized");
	
	// 채널명 있는지 확인
	Channel *channel = server->getChannelByChannelName(token[2].substr(1));
	if (channel == NULL)
		throw std::runtime_error("Channel doesn't exist");
	
	// 입력 유저가 channel에 있는지, operator인지 확인
	if (channel->checkClientExistByClientFd(client->getFd()) == 0)
		throw std::runtime_error("User is not in channel");
	if (channel->isOperator(client->getFd()) == 0)
		throw std::runtime_error("User is not operator");
	
	// kickClients (token[3] : nickname list)
	std::vector<std::string> kickClients = split(token[3], ",");
	for (size_t i = 0; i < kickClients.size(); ++i) {
   		std::cout << "kickClient: " << kickClients[i] << " ";
	}
	std::cout << std::endl;

	std::vector<std::string>::iterator it;
	for (it = kickClients.begin(); it != kickClients.end(); ++it)
	{
		Client* kickClient = channel->getClientByNickname(*it);
		if (!kickClient)
			continue ;
		channel->removeClient(kickClient->getFd());
		kickClient->removeChannel(token[2].substr(1));
		std::cout << "kick " << kickClient->getFd() << " from " << token[2] << std::endl;
	}

	std::string message = "There is no reason";
	if (token.size() == 5)
		message = client->getNickName() + ": " + token[4].substr(1) + '\n';	
	channel->broadcast(message);
}

void	Command::invite(Server *server, Client *client, std::string nickName, std::string channelName)
{
	if (!client->getAuthorized())
		throw std::runtime_error("Not authorized");
	
	// 채널명 있는지 확인
	Channel *channel = server->getChannelByChannelName(channelName);
	if (channel == NULL)
		throw std::runtime_error("Channel doesn't exist");
	
	// 입력 유저가 channel에 있는지, operator인지 확인
	if (channel->checkClientExistByClientFd(client->getFd()) == 0)
		throw std::runtime_error("User is not in channel");
	if (channel->isOperator(client->getFd()) == 0)
		throw std::runtime_error("User is not operator");
	
	std::cout << "channel client count: " << channel->getClientList().size() << std::endl;
	Client* inviteClient = server->getClientByNickname(nickName);
	if (!inviteClient)
		return ;
	channel->addClient(inviteClient);
	std::cout << "channel client count: " << channel->getClientList().size() << std::endl;
}

void	Command::topic(Server *server, Client *client, std::vector<std::string> token)
{
	if (!client->getAuthorized())
		throw std::runtime_error("Not authorized");
	
	// 채널명 있는지 확인
	Channel *channel = server->getChannelByChannelName(token[2].substr(1));
	if (channel == NULL)
		throw std::runtime_error("Channel doesn't exist");
	
	// 입력 유저가 channel에 있는지, operator인지, channel mode에 't' 있는지 확인
	if (channel->checkClientExistByClientFd(client->getFd()) == 0)
		throw std::runtime_error("User is not in channel");
	if (channel->isOperator(client->getFd()) == 0)
		throw std::runtime_error("User is not operator");
	if (channel->getMode().find('t') == std::string::npos)
		throw std::runtime_error("Channel doesn't have permission to set topic");
	
	if (token.size() == 3)
		std::cout << channel->getTopic() << std::endl;
	else
	{
		if (token[3][0] != ':')
			throw std::runtime_error("Invalid argument");
		std::string topic = token[3].substr(1);
		if (topic.empty())
			channel->clearTopic();
		else
			channel->setTopic(topic);
	}
}

void Command::mode(Server *server, Client *client, std::vector<std::string> token)
{
	if (token[2][0] != '#')
		throw std::runtime_error("Invalid argument");
	if (token[3][0] != '+' && token[3][0] != '-')
		throw std::runtime_error("Invalid argument");
	if (token[3][1] != 'i' && token[3][1] != 't' && token[3][1] != 'k' && token[3][1] != 'o' && token[3][1] != 'l')
		throw std::runtime_error("Invalid argument");
	if (token[3].length() != 2)
		throw std::runtime_error("Invalid argument");
	
	if (!client->getAuthorized())
		throw std::runtime_error("Not authorized");
	
	// 채널명 있는지 확인
	Channel *channel = server->getChannelByChannelName(token[2].substr(1));
	if (channel == NULL)
		throw std::runtime_error("Channel doesn't exist");
	
	// 입력 유저가 channel에 있는지, operator인지 확인
	if (channel->isOperator(client->getFd()) == 0)
		throw std::runtime_error("User is not operator");
	if (token[3][1] == 'o')
		if (channel->getClientByNickname(token[4]) == NULL)
			throw std::runtime_error("User is not in channel");
	

	if (token[3][1] == 'i')
	{
		// invite 동작 추가하기
		// if (token[3][0] == '+')
		// 	channel->setMode('i');
		// else
		// 	channel->removeMode('i');
	}
	else if (token[3][1] == 't')
	{
		if (token[3][0] == '+')
			channel->setMode('t');
		else
			channel->removeMode('t');
	}
	if (token[3][1] == 'k')
	{
		if (token[3][0] == '+')
			channel->setKey(token[4]);
		else
		{
			if (token.size() != 4)
				throw std::runtime_error("Invalid argument");
			channel->setKey("");
		}
	}
	else if (token[3][1] == 'o')
	{
		Client *opClient = channel->getClientByNickname(token[4]);
		if (token[3][0] == '+')
			channel->_operatorList[opClient->getFd()] = opClient;
		else
			channel->_operatorList.erase(opClient->getFd());
	}
	// (1/0) MODE #channel +l/-l (10)
	else if (token[3][1] == 'l')
	{
		if (token[3][0] == '+')
		{
			channel->setMode('l');
			int limit = atoi(token[4].c_str());
			if (limit < 0)
				return;
			channel->setLimit(limit);
		}
		else
		{
			if (token.size() != 4)
				throw std::runtime_error("Invalid argument");
			channel->removeMode('l');
			channel->setLimit(-1);
		}
	}
}

std::vector<std::string>	Command::split(std::string str, std::string delimiter) {
	std::vector<std::string> ret;

	size_t pos = 0;
	while ((pos = str.find(delimiter)) != std::string::npos)
	{
		ret.push_back(str.substr(0, pos));
		str.erase(0, pos + delimiter.length());
	}
	if (!str.empty())
		ret.push_back(str);
	return ret;
}
