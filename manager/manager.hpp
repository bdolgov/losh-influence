#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include <QDebug>

#define TIMEOUT 1000

struct Coord
{
	int x, y;
	Coord(int _x, int _y): x(_x), y(_y) {}
	bool operator<(const Coord& other) const
	{
		return x != other.x ? x < other.x : y < other.y;
	}
	friend std::ostream& operator<<(std::ostream& o, const Coord &c);
};

class Player;
class Cell;
class PlayerEngine;

struct LogicError : std::logic_error
{
	LogicError(): std::logic_error("Game logic error") {}
};

class Player
{
	public:
		enum Status { Waiting, Fortify, Conquest, Broken, Lost };
		Status status() const;
		void setStatus(Status status);

		int id() const { return m_id; }

		bool needHello();
		void failed();

		PlayerEngine *engine() const;

		friend class Game;
		friend std::ostream& operator<<(std::ostream& o, Player *p);
		
	private:
		int m_id;
		PlayerEngine *m_engine;
		Status m_status = Waiting;
		bool sentHello = false;
		Player(int _id, PlayerEngine* _engine): m_id(_id), m_engine(_engine) {}
		int n_failed = 0;
		int fortificationLimit = 0;
		int sequentalMaps = 0;
};

class Cell
{
	public:
		Coord coord;
		std::vector<Cell*> neigh;
		Player *owner = nullptr;
		int size = 0;
		int power = 0;

		bool visibleTo(Player *player);

		Cell(Coord _coord, int _size);
};

class Game
{
	private:
		std::map<Coord, std::unique_ptr<Cell>> cells;
		std::vector<std::unique_ptr<Player>> players;
		int currentPlayer = 0;

		Cell* findCell(int x, int y);
		Player *m_winner = nullptr;

	public:
		enum AttackResult { Null, Won, Draw, Lost };
		void addPlayer(PlayerEngine* player);

		void addPower(Player* player, Cell* cell);
		Game::AttackResult attack(Player* player, Cell* cell, Cell* other);

		void processPlayer(Player* player);
		bool gameFinished();

		void loadMap(const std::string& path);
		void step();

		std::vector<Cell*> getCells() const;

		Player *winner();
};

class PlayerEngine
{
	public:
		struct Command
		{
			enum Type { Null, BeginConquest, Conquest, BeginFortification, Fortify, Map, Pass } type = Null;
			std::vector<int> args;
		};
		
		virtual ~PlayerEngine() {}

		virtual void sendHello(int id) = 0;
		virtual void sendError() = 0;
		virtual void sendOk() = 0;
		virtual void sendConquestResult(Game::AttackResult result, int pa, int pb) = 0;
		virtual void sendGameResult(int result) = 0;
		virtual void sendMap(const std::vector<Cell*>& cells, const std::vector<int>& players) = 0;
		virtual Command getCommand() = 0;
};

class LogWriter
{
	public:
		virtual void addCell(Cell* cell) = 0;
		virtual void addComment(const std::string& s) = 0;
		virtual ~LogWriter() {}
};

class Log
{
	private:
		class LogStream
		{
			private:
				Log& logger;
				std::shared_ptr<std::stringstream> record;

			public:
				LogStream(Log& _logger);
				~LogStream();

				template<class T>
				LogStream& operator<<(const T& value)
				{
					(*record) << value;
					return *this;
				}
		};
		static LogWriter* logWriter;

	public:
		Log();
		LogStream operator()();
		void addCell(Cell* cell);
		void addComment(const std::string& s);
		static void setLogWriter(LogWriter* lw) { logWriter = lw; }
};

class TextPlayer : public PlayerEngine
{
	public:
		virtual void sendHello(int id);
		virtual void sendError();
		virtual void sendOk();
		virtual void sendConquestResult(Game::AttackResult result, int pa, int pb);
		virtual void sendGameResult(int result);
		virtual void sendMap(const std::vector<Cell*>& cells, const std::vector<int>& players);
		virtual Command getCommand();
	protected:
		virtual void write(const std::string& s) = 0;
		virtual std::string read() = 0;
};

class StdioPlayer : public TextPlayer
{
	protected:
		void write(const std::string& s);
		std::string read();
};

#ifdef QT_CORE_LIB
#include <QIODevice>
#include <QTimer>
#include <QObject>
#include <QEventLoop>
#include <QDateTime>

class QIODevicePlayer : public TextPlayer
{
	public:
		QIODevicePlayer(QIODevice *_io, int _timeout = 1000): io(_io), timeout(_timeout) {}

	protected:
		void write(const std::string& s)
		{
			qDebug() << "WRITING" << s.c_str();
			io->write((s + "\n").c_str());
			qDebug() << "waitForBytesWritten" << io->waitForBytesWritten(timeout);
		}

		std::string read()
		{
			QDateTime now = QDateTime::currentDateTime(), timeout = now.addMSecs(this->timeout);
			bool ret = true;
			while (!io->canReadLine() && ret)
			{
				now = QDateTime::currentDateTime();
				ret = io->waitForReadyRead(now.msecsTo(timeout));
				qDebug() << ret;
			}
			if (!io->canReadLine())
			{
				//qDebug() << "waitForReadyRead" << io->waitForReadyRead(1000);
				/*QEventLoop evloop;
				QTimer timer;
				timer.start(TIMEOUT);
				QObject::connect(&timer, &QTimer::timeout, &evloop, &QEventLoop::quit);
				QObject::connect(io, &QIODevice::readyRead, &evloop, &QEventLoop::quit);
				evloop.exec();*/
			}
			if (!io->canReadLine())
			{
				return "";
			}
			else
			{
				return io->readLine().data();
			}
		}

	private:
		QIODevice *io;
		int timeout;
};
#endif

class StderrLogWriter : public LogWriter
{
	public:
		void addCell(Cell*);
		void addComment(const std::string& s);
};


extern Log logger;
extern bool logMap;

#endif
