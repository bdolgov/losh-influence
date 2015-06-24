//#include <QtCore>
#include "manager.hpp"

#include <random>
#include <iostream>
#include <fstream>


#define FAILS 100

bool logMap = false;

Log logger;
LogWriter* Log::logWriter = nullptr;

Cell::Cell(Coord _coord, int _size):
	coord(_coord), size(_size)
{
}

Player::Status Player::status() const
{
	return m_status;
}

void Player::setStatus(Player::Status _status)
{
	if ((m_status == Waiting && _status != Conquest)
		|| (m_status == Conquest && _status != Fortify)
		|| (m_status == Fortify && _status != Waiting))
	{
		logger() << this << " пытается сменить состояние на неразрешенное";
		throw LogicError();
	}
	m_status = _status;
}

void Player::failed()
{
	++n_failed;
	if (n_failed == FAILS)
	{
		logger() << this << " отправил слишком много ошибочных команд и дисквалифицирован.";
		m_status = Broken;
	}
}

void Game::loadMap(const std::string& path)
{
	if(players.empty())
	{
		throw std::logic_error("You must add players before loading the map!");
	}
	std::ifstream f(path);
	int n; f >> n;
	for (int i = 0; i < n; ++i)
	{
		int x, y, s, m;
		f >> x >> y >> s >> m;
		cells.insert(std::make_pair(Coord(x, y), std::unique_ptr<Cell>(new Cell({x, y}, s))));
		for (int j = 0; j < m; ++j)
			f >> x >> y;
	}
	f.seekg(0);
	f >> n;
	for (int i = 0; i < n; ++i)
	{
		int x, y, s, m;
		f >> x >> y >> s >> m;
		Cell *c = findCell(x, y);
		for (int j = 0; j < m; ++j)
		{
			f >> x >> y;
			c->neigh.push_back(findCell(x, y));
		}
	}
	for (auto &i : players)
	{
		int x, y;
		f >> x >> y;
		findCell(x, y)->owner = &*i;
		findCell(x, y)->power = 2;
	}
}

void Game::addPlayer(PlayerEngine *player)
{
	if (!cells.empty())
	{
		throw std::logic_error("You must not add players after loading the map!");
	}
	players.emplace_back(new Player(players.size() + 1, player));
}

bool Cell::visibleTo(Player* player)
{
	if (owner == player) return true;
	for (auto i : neigh)
	{
		if (i->owner == player) return true;
	}
	return false;
}

PlayerEngine *Player::engine() const
{
	return m_engine;
}

std::ostream& operator<<(std::ostream& o, const Coord &c)
{
	o << c.x << " " << c.y;
	return o;
}

bool Player::needHello()
{
	if (sentHello)
		return false;
	sentHello = true;
	return true;
}

std::ostream& operator<<(std::ostream& o, Player *p)
{
	if (!p)
	{
		o << "Player (null)";
	}
	else
	{
		o << "Player " << p->id();
	}
	return o;
}

void TextPlayer::sendHello(int id)
{
	write("HELLO " + std::to_string(id));
}

void TextPlayer::sendError()
{
	write("ERROR");
}

void TextPlayer::sendOk()
{
	write("OK");
}

void TextPlayer::sendConquestResult(Game::AttackResult result, int pa, int pb)
{
	write((result == Game::Won ? "SUCCESS " : "FAIL ") + std::to_string(pa) + " " + std::to_string(pb));
}

void TextPlayer::sendGameResult(int result)
{
	if (result < 0)
		write("LOST");
	else if (result == 0)
		write("CONTINUE");
	else 
		write("WON");
}

void TextPlayer::sendMap(const std::vector<Cell*>& cells, const std::vector<int>& players)
{
	std::stringstream text;
	text << cells.size() << "\n";
	for (auto i : cells)
	{
		text << i->coord << " " << i->size << " " << (i->owner ? i->owner->id() : 0) << " " << i->power << " " << i->neigh.size() << "\n";
		for (auto j : i->neigh)
		{
			text << j->coord << "\n";
		}
	}
	text << players[1] << " " << players[2] << " " << players[3] << " " << players[4];
	std::string ret = text.str();
	if (logMap) logger() << "Ответ на запрос карты:\n" << ret;
	write(ret);
}

void StdioPlayer::write(const std::string& s)
{
	std::cout << s + "\n";
}

std::string StdioPlayer::read()
{
	std::string incoming;
	std::getline(std::cin, incoming);
	return incoming;
}

PlayerEngine::Command TextPlayer::getCommand()
{
	std::string incoming = read();
	if (incoming.empty())
		return Command();
	if (*(incoming.rbegin()) == '\n')
	{
		incoming.resize(incoming.size() - 1);
	}
	logger() << "Принята команда " << incoming; 
	std::stringstream ss(incoming);
	Command ret;
	std::string cmd;
	ss >> cmd;
	if (cmd == "BEGIN")
	{
		ss >> cmd;
		if (cmd == "CONQUEST")
		{
			ret.type = ret.BeginConquest;
		}
		else if (cmd == "FORTIFICATION")
		{
			ret.type = ret.BeginFortification;
		}
		else
		{
			logger() << "Неизвестная команда";
			throw LogicError();
		}
	}
	else if (cmd == "MAP")
	{
		ret.type = ret.Map;
	}
	else if (cmd == "CONQUEST")
	{
		ret.type = ret.Conquest;
		ret.args.resize(4);
		ss >> ret.args[0] >> ret.args[1] >> ret.args[2] >> ret.args[3];
	}
	else if (cmd == "FORTIFY")
	{
		ret.type = ret.Fortify;
		ret.args.resize(2);
		ss >> ret.args[0] >> ret.args[1];
	}
	else if (cmd == "PASS")
	{
		ret.type = ret.Pass;
	}
	else
	{
		logger() << "Неизвестная команда";
		throw LogicError();
	}
	return ret;
}


int getAttack(int power)
{
	static std::mt19937 generator;
	static std::uniform_int_distribution<int> distribution(1,6);
	int ret = 0;
	for (int i = 0; i < power; ++i)
		ret += distribution(generator);
	return ret;
}

Log::LogStream::LogStream(Log& _logger):
	logger(_logger),
	record(new std::stringstream)
{
}

Log::LogStream::~LogStream()
{
	logger.addComment(record->str());
}

Log::Log()
{
	if (!logWriter)
	{
		logWriter = new StderrLogWriter();
	}
}

Log::LogStream Log::operator()()
{
	return LogStream(*this);
}

void Log::addCell(Cell* cell)
{
	logWriter->addCell(cell);
}

void Log::addComment(const std::string& s)
{
	logWriter->addComment(s);
}

void Game::addPower(Player* player, Cell* cell)
{
	logger() << player << " пытается добавить силу в клетку " << cell->coord;
	if (player->status() != player->Fortify)
	{
		logger() << "Ошибка: игрок не находится в стадии укрепления";
		throw LogicError();
	}
	if (cell->owner != player)
	{
		logger() << "Ошибка: клеткой владеет " << cell->owner;
		throw LogicError();
	}
	if (cell->power == cell->size)
	{
		logger() << "Ошибка: клетка превысит свой размер " << cell->size;
		throw LogicError();
	}
	++cell->power;
	logger.addCell(cell);
	logger() << "Успешно: теперь сила клетки равна " << cell->power << " / " << cell->size;
}

Game::AttackResult Game::attack(Player* player, Cell* cell, Cell* other)
{
	logger() << player << " пытается атаковать клеткой " << cell->coord << " клетку " << other->coord;
	if (player->status() != player->Conquest)
	{
		logger() << "Ошибка: игрок не находится в стадии атаки";
		throw LogicError();
	}
	if (player != cell->owner)
	{
		logger() << "Ошибка: атакующей клеткой владеет другой игрок " << cell->owner;
		throw LogicError();
	}
	if (cell->power < 2)
	{
		logger() << "Ошибка: сила клетки меньше 2";
		throw LogicError();
	}
	if (std::find(cell->neigh.begin(), cell->neigh.end(), other) == cell->neigh.end())
	{
		logger() << "Ошибка: атакующая и атакуемая клетка не являются соседями";
		throw LogicError();
	}
	if (other->owner == player)
	{
		logger() << "Ошибка: атакующий игрок владеет атакуемой клеткой";
		throw LogicError();
	}
	AttackResult result;
	if (!other->owner)
	{
		logger() << "Успешно: захвачена ничья клетка";
		other->owner = cell->owner;
		other->power = std::min(cell->power - 1, other->size);
		cell->power = 1;
		result = Won;
	}
	else
	{
		logger() << "Атакумая клетка принадлежит игроку " << other->owner;
		int attackA = getAttack(cell->power), attackB = getAttack(other->power);
		logger() << "Атакующая клетка: " << attackA << ", атакуемая клетка: " << attackB;
		if (attackA > attackB)
		{
			logger() << "Атакующий побеждает";
			other->owner = cell->owner;
			other->power = std::min(std::max(1, cell->power - (attackB + 5) / 6 - 1), other->size);
			cell->power = 1;
			result = Won;
		}
		else if (attackA == attackB)
		{
			logger() << "Ничья!";
			cell->power = 1;
			other->power = 1;
			result = Draw;
		}
		else
		{
			logger() << "Атакуемый побеждает";
			cell->power = 1;
			other->power = std::max(1, other->power - (attackA + 5) / 6);
			result = Lost;
		}
	}
	logger.addCell(cell);
	logger.addCell(other);
	return result;
}

Player *Game::winner()
{
	return m_winner;
}

void Game::processPlayer(Player* player)
{
	if (player->status() == player->Broken)
	{
		logger() << "Ход " << player << " пропускается, так как он дисквалифицирован";
		return;
	}
	if (player->status() == player->Lost)
	{
		logger() << "Ход " << player << " пропускается, так как он проиграл";
		return;
	}
	if (player->status() != player->Waiting)
	{
		logger() << "Внутреннаяя ошибка: processPlayer при статусе " << static_cast<int>(player->status());
		return;
		//throw std::runtime_error("RUNTIME ERROR");
	}
	PlayerEngine *pe = player->engine();
	int cnt = 0, cntOther = 0;
	for (auto &i : cells)
	{
		if (i.second->owner == player)
		{
			++cnt;
		}
		else if (i.second->owner)
		{
			++cntOther;
		}
	}
	if (!cnt)
	{
		logger() << "Игрок " << player << " проиграл";
		player->m_status = player->Lost;
		pe->sendGameResult(-1);
		return;
	}
	if (!cntOther)
	{
		logger() << "Игрок " << player << " победил";
		m_winner = player;
		pe->sendGameResult(1);
		return;
	}
	logger() << "Начинается ход " << player;
	
	if (player->needHello())
	{
		pe->sendHello(player->id());
	}
	else
	{
		pe->sendGameResult(0);
	}
	while (player->status() != player->Broken)
	{
		try
		{
			auto cmd = pe->getCommand();
			if (cmd.type != cmd.Map)
			{
				player->sequentalMaps = 0;
			}
			if (cmd.type == cmd.Null)
			{
				logger() << player << " дисквалифицирован в связи с остановкой программы";
				player->m_status = player->Broken;
				break;
			}
			else if (cmd.type == cmd.Map)
			{
				logger() << player << " запрашивает карту";
				if (++player->sequentalMaps > 5)
				{
					logger() << "Ошибка: запросил карту больше 5 раз подряд";
					throw LogicError();
				}
				std::vector<int> pplayers(5);
				std::vector<Cell*> pcells;
				for (auto &i : cells)
				{
					if (i.second->visibleTo(player))
					{
						pcells.push_back(&*(i.second));
					}
					if (i.second->owner)
					{
						++pplayers[i.second->owner->id()];
					}
				}
				pe->sendMap(pcells, pplayers);
			}
			else if (cmd.type == cmd.BeginConquest)
			{
				player->setStatus(player->Conquest);
				pe->sendOk();
				logger() << player << " перешел в режим завоевания";

			}
			else if (cmd.type == cmd.BeginFortification)
			{
				player->setStatus(player->Fortify);
				player->fortificationLimit = 0;
				for (auto &i : cells)
				{
					if (i.second->owner == player)
						++player->fortificationLimit;
				}
				player->fortificationLimit -= turn == player->id();
				qDebug() << "turn" << turn << "pid+1" << player->id();
				pe->sendOk();
				logger() << player << " перешел в режим укрепления. Лимит: " << player->fortificationLimit;
			}
			else if (cmd.type == cmd.Pass)
			{
				player->setStatus(player->Waiting);
				break;
			}
			else if (cmd.type == cmd.Fortify)
			{
				if (!player->fortificationLimit)
				{
					logger() << player << " больше не может укреплять свои клетки";
					throw LogicError();
				}
				addPower(player, findCell(cmd.args[0], cmd.args[1]));
				pe->sendOk();
				--player->fortificationLimit;
				logger() << player << " может выполнить еще " << player->fortificationLimit << " улучшений";
			}
			else if (cmd.type == cmd.Conquest)
			{
				Cell *a = findCell(cmd.args[0], cmd.args[1]), *b = findCell(cmd.args[2], cmd.args[3]);
				auto result = attack(player, a, b);
				pe->sendConquestResult(result, a->power, b->power);
			}
			else if (cmd.type == cmd.Pass)
			{
				player->setStatus(player->Waiting);
			}
		}
		catch (LogicError&)
		{
			logger() << player << ": команда завершилась ошибкой";
			player->failed();
			pe->sendError();
		}
	}
}

Cell* Game::findCell(int x, int y)
{
	auto it = cells.find({x, y});
	if (it != cells.end()) return &*(it->second);
	logger() << "Клетка с координатами " << x << " " << y << " не существует!";
	throw LogicError();
}

void Game::step()
{
	processPlayer(&*players[currentPlayer]);
	currentPlayer = (currentPlayer + 1) % players.size();
	if (currentPlayer == 0)
		++turn;
}

void StderrLogWriter::addCell(Cell*)
{
}

void StderrLogWriter::addComment(const std::string& s)
{
	std::cerr << s << std::endl;
}

std::vector<Cell*> Game::getCells() const
{
	std::vector<Cell*> ret;
	for (auto &i : cells)
	{
		ret.emplace_back(&*i.second);
	}
	return ret;
}
