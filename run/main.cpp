#include "../manager/manager.hpp"
#include <QtCore>
#include <map>
#include <iterator>
#include <fstream>
#include <iostream>

class Logger : public LogWriter
{
	public:
		Logger(std::ostream& _stream);
		void addComment(const std::string& s);
		void addCell(Cell* c);

	private:
		std::ostream &stream;
};

Logger::Logger(std::ostream& _stream):
	stream(_stream)
{
}

void Logger::addComment(const std::string& s)
{
	stream << "COMMENT " << s << std::endl;
}

void Logger::addCell(Cell *c)
{
	stream << "CELL " << c->coord << " " << c->power << " " << c->owner->id() << std::endl;
}

int main(int ac, char** av)
{
	QCoreApplication app(ac, av);

	Game game;

	int steps = 100;

	if (ac < 2)
	{
		qDebug() << "Usage: run <config>";
		return 1;
	}
	std::ifstream _configFile(av[1]);
	std::istream& configFile = std::string(av[1]) == "-" ? std::cin : _configFile;
	std::string cmd, map, logFileName;
	std::vector<std::pair<std::string, std::string>> players;
	while (configFile >> cmd)
	{
		if (cmd == "MAP")
			configFile >> map;
		else if (cmd == "LOG")
			configFile >> logFileName;
		else if (cmd == "STEPS")
			configFile >> steps;
		else if (cmd == "PLAYER")
		{
			std::string p, n;
			configFile >> p;
			while (configFile.peek() == ' ')
				configFile.ignore(1);
			getline(configFile, n);
			players.emplace_back(p, n);
		}
	}
	std::vector<QProcess*> procs;

	for (auto& i : players)
	{
		QProcess *p = new QProcess;
		procs.push_back(p);
		p->setWorkingDirectory("trash");
		p->start(QString::fromStdString(i.first));
		if (!p->waitForStarted())
		{
			qDebug() << p->errorString();
			return 1;
		}
		game.addPlayer(new QIODevicePlayer(p, 100), i.second);
	}

	std::ifstream mapFile(map);

	std::ofstream logFile(logFileName);

	logFile << "MAP ";
	std::copy(std::istream_iterator<int>(mapFile), std::istream_iterator<int>(), std::ostream_iterator<int>(logFile, " "));
	logFile << std::endl;

	Log::setLogWriter(new Logger(logFile));

	game.loadMap(map);

	for (int i = 0; i < steps && !game.gameFinished(); ++i)
	{
		logFile << "STEP" << std::endl;
		game.step();
	}

	if (game.winner())
	{
		std::cout << game.winner()->id() << " WIN" << std::endl;
	}
	else
	{
		std::vector<int> results(players.size() + 1);
		for (auto i : game.getCells())
		{
			if (i->owner)
			{
				++results[i->owner->id()];
			}
		}

		std::cout << (std::max_element(results.begin(), results.end()) - results.begin()) << " STEPS" << std::endl;
	}

	for (auto i : procs)
	{
		i->kill();
		if (!i->waitForFinished(1000))
		{
			qDebug() << "Process" << i << "did not finish";
		}
		delete i;
	}

	return 0;
}
