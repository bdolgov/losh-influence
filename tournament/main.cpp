#include <QtCore>
#include <algorithm>
#include <fstream>
#include <string>
#include <future>
#include <vector>

#define FUTURES

using namespace std;
class Player;
struct Game
{
	int id;
	std::string reason, map;
	struct Participation
	{
		Player *p;
		bool won;
		double oldRating, newRating;
	} p[4];
};

class Player
{
	public:
		int id;
		QString name;
		double score = 1500;
		int wins = 0;
		Player(int a, QString b): id(a), name(b) {}
		bool operator<(const Player& other) const { return score > other.score; }
		vector<Game*> games;
		double maxrating = 0;
};

vector<Player> _players = {
{15719, "Егор Лифарь"},
{181663, "Иван Иванов"},
{23097, "Глеб Скиба"},
{24376, "Егор Поляков"},
{26364, "Максим Плюшкин"},
{27172, "Artemiy Vishnjakov"},
{33219, "Николай Лавренов"},
{34673, "Роман Логинов"},
{35094, "Павел Труфанов"},
{35354, "Илья Воронин"},
{35768, "Михаил Филимонов"},
{36513, "Алексей Соловьёв"},
{36722, "Михаил   Ситников"},
{37212, "Дмитрий Лунин"},
{37485, "Василий Алфёров"},
{40056, "Максим Еремеев"},
{45473, "Пётр Григорьев"},
{45859, "Виктория Чигур"},
{46813, "Богдан Синицын"},
{487, "Изи Экзэ"},
{53837, "Николай Оплачко"},
{54823, "Ярослав Гераськин"},
{55339, "Георгий Халин"},
{55489, "Владимир Романов"},
{56645, "Федя Куянов"},
{56912, " Артём Рябов"},
{57499, "Филипп Грибов"},
{57504, "Евгений Коган"},
{57513, "Вадим Павлов"},
{59100, "Михаил Сорокин"},
{60328, "Алексей Глазкин"},
{60437, "Андрей Календаров"},
{61759, "Андрей Недолужко"},
{63097, "Максим  Зимнюков"},
{63098, "Борис Фаизов"},
{67048, "Михаил Прохоров"},
{67131, "Данил Гейдебрехт"},
{67144, "Евгений Лупашин"},
{70153, "Григорий Резников"},
{71638, "Василий Новак"},
{72914, "Николай Карташев"},
{75207, "Denis Tsyupa"},
{76083, "Егор Чунаев"},
{76515, "Ален Алиев"},
{80518, "Сергей Алейкин"},
{81377, "Иван Лозинский"},
{83507, "Василий Морозов"},
{88082, "Дмитрий Саютин"},
{90509, "Антон Алёшин"},
{92558, "Азамат Мифтахов"},
{95704, "Борис Соболев"},
{101, "Сломал ноутбук"},
{102, "Данил Гейдебрехт"},
{103, "Алексей Глазкин"},
{104, "Матвей Грицаев"},
{105, "Павлов"},
{97385, "Артем Переведенцев"}};

void process(QString text)
{
	static QMutex mutex;
	QMutexLocker l(&mutex);
	Q_UNUSED(l);
	qDebug() << text;
}

int main(int ac, char** av)
{
	int play_id = 0;
	vector<Player*> players;
	for (auto &i : _players)
	{
		players.push_back(&i);
	}
	for (int s = 0; s < 400; ++s)
	{
		random_shuffle(players.begin(), players.end());
		if (s % 5 == 4)
			sort(players.begin(), players.end(), [](Player* a, Player* b) { return *a < *b; });
		vector<future<void>> futures;
		for (int i = 0; i + 3 < players.size(); i += 4)
		{
			++play_id;
#ifdef FUTURES
			futures.push_back(async(launch::async, [i, play_id, &players]() { do {
#endif
			Game *me = new Game;
			me->id = play_id;
			int map = rand() % 4 + 1;
			me->map = std::to_string(map);
			QString cfg = QString("MAP /root/losh-influence/maps/t2map0%1.txt\n"
					"PLAYER /root/exe/%2 %3\n"
					"PLAYER /root/exe/%4 %5\n"
					"PLAYER /root/exe/%6 %7\n"
					"PLAYER /root/exe/%8 %9\n"
					"LOG output/%10.log\n"
					"STEPS 512\n")
				.arg(map)
				.arg(players[i]->id).arg(players[i]->name)
				.arg(players[i+1]->id).arg(players[i+1]->name)
				.arg(players[i+2]->id).arg(players[i+2]->name)
				.arg(players[i+3]->id).arg(players[i+3]->name)
				.arg(me->id);
			QProcess run;
			run.start("../run/run", {"-"});
			run.waitForStarted(60000);
			run.write(cfg.toUtf8());
			run.closeWriteChannel();
			run.waitForFinished(600000);
			QByteArray ret = run.readAll();
			QTextStream ts(&ret);
			QString reason;
			int w = 0; ts >> w >> reason;
			me->reason = reason.toStdString();
			if (!w) continue;
			++players[i + w - 1]->wins;
			for (int j = 0; j < 4; ++j)
			{
				me->p[j].oldRating = players[i + j]->score;
				me->p[j].p = players[i + j];
			}
			for (int j = 0; j < 4; ++j)
			{
				for (int k = j + 1; k < 4; ++k)
				{
					double Ea1 = 1.0 / (1 + pow(10, (players[i + k]->score - players[i + j]->score) / 400.0));
					double Ea2 = 1.0 / (1 + pow(10, (players[i + j]->score - players[i + k]->score) / 400.0));
					double sa = 0.5, sb = 0.5;
					double K = 20;
					if (w - 1 == j)
						sa = 1, sb = 0;
					else if (w - 1 == k)
						sa = 0, sb = 1;
					else
						K = 0;
					players[i + j]->score += K * (sa - Ea1);
					players[i + k]->score += K * (sb - Ea2);
				}
			}
			for (int j = 0; j < 4; ++j)
			{
				me->p[j].newRating = players[i + j]->score;
				me->p[j].won = w - 1 == j;
				players[i + j]->games.push_back(me);
			}
#ifdef FUTURES
			} while(0); }));
#endif
		}
#ifdef FUTURES
		for (auto &i : futures)
		{
			i.get();
		}
#endif
	}
	for (auto &i : players)
	{
		i->score = 0;
		for (auto j = i->games.rbegin(); j != i->games.rbegin() + i->games.size() / 4; ++j)
		{
			for (auto k : (*j)->p)
			{
				if (k.p == i)
				{
					i->score += k.newRating;
					i->maxrating = max(i->maxrating, k.newRating);
				}
			}
		}
		i->score /= i->games.size() / 4;
	}
	sort(players.begin(), players.end(), [](Player* a, Player* b) { return *a < *b; });
	ofstream full("output/full_res.json");
	full << "[";
	int place = 1;
	for (auto i : players)
	{
		if (place != 1)
		{
			full << ",";
		}
		full << "{" 
				<< "\"name\":\"" << i->name.toStdString() << "\"," 
				<< "\"player_id\":" << i->id << ","
				<< "\"rating\":" << i->score << ","
				<< "\"won\":" << i->wins << ","
				<< "\"total\":" << i->games.size() << ","
				<< "\"maxrating\":" << i->maxrating
			<< "}";
		ofstream p(QString("output/player_%1_res.json").arg(i->id).toStdString());
		p << "{"
			<< "\"name\":\"" << i->name.toStdString() << "\","
			<< "\"player_id\":" << i->id << ","
			<< "\"rating\":" << i->score << ","
			<< "\"place\":" << place << ","
			<< "\"won\":" << i->wins << ","
			<< "\"maxrating\":" << i->maxrating << ","
			<< "\"games\":[";
		bool first = true;
		for (auto j : i->games)
		{
			if (!first)
			{
				p << ",";
			}
			first = false;
			p << "{"
				<< "\"game_id\":" << j->id << ","
				<< "\"reason\":\"" << j->reason << "\","
				<< "\"map\":\"" << j->map << "\","
				<< "\"players\":[";
			bool first2 = true;
			for (auto k : j->p)
			{
				if (!first2)
				{
					p << ",";
				}
				first2 = false;
				p << "{"
					<< "\"player_id\":" << k.p->id << ","
					<< "\"name\":\"" << k.p->name.toStdString() << "\","
					<< "\"old_rating\":" << k.oldRating << ","
					<< "\"new_rating\":" << k.newRating << ","
					<< "\"won\":" << (k.won ? "true" : "false")
					<< "}";
			}
			p << "]}";
		}
		p << "]}";
		++place;
	}
	full << "]";
}
