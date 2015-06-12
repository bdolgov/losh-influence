#include "ui.hpp"
#include "manager.hpp"

int main(int ac, char** av)
{
	QApplication app(ac, av);

	StartupUi ui;
	ui.show();

	return app.exec();
}

StartupUiPlayer::StartupUiPlayer(int _id, StartupUi *_ui):
	id(_id), ui(_ui)
{
	addItem("None", "__none__");
	addItem("Human", "__human__");
	addItem("Program", "__program__");
	path = "__none__";
	QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idxChanged(int)));
	setWindowTitle("Influence configuration");
}

void StartupUiPlayer::idxChanged(int idx)
{
	if (id + 1 < ui->players.size())
	{
		if (idx == 0)
			ui->players[id + 1]->setCurrentIndex(0);
		ui->players[id + 1]->setEnabled(idx != 0);
	}
	
	if (idx == 0)
		path = "__none__";
	else if (idx == 1)
		path = "__human__";
	else if (idx == 2)
	{
		path = QFileDialog::getOpenFileName(ui, "Select executable");
		if (path == "")
			setCurrentIndex(0);
	}
}

StartupUi::StartupUi()
{
	setWindowTitle("Influence");
	QFormLayout *l = new QFormLayout;
	for (unsigned i = 0; i < 4; ++i)
	{
		StartupUiPlayer *p = new StartupUiPlayer(i, this);
		if (i)
			p->setEnabled(false);
		players.push_back(p);
		l->addRow(QString("Player %1").arg(i + 1), p);
	}
	QPushButton *loadMapB = new QPushButton("Load...");
	l->addRow("Map", loadMapB);
	QObject::connect(loadMapB, SIGNAL(clicked()), this, SLOT(loadMap()));
	QPushButton *startB = new QPushButton("Start");
	l->addRow(startB);
	QObject::connect(startB, SIGNAL(clicked()), this, SLOT(start()));
	setLayout(l);
	setWindowTitle("Influence");
}

void StartupUi::loadMap()
{
	map = QFileDialog::getOpenFileName(this, "Select map");
}

void StartupUi::start()
{
	if (map == "" || players[0]->path == "__none__")
	{
		QMessageBox::critical(this, "FATAL ERROR", "You must select at least one player and load map!");
		return;
	}
	QStringList players0;
	for (auto i : players)
	{
		if (i->path != "__none__")
		{
			players0.append(i->path);
		}
	}
	GameUi *gui = new GameUi(map, players0);
	gui->show();
	hide();
}

HumanPlayer::HumanPlayer()
{
	QGridLayout *l = new QGridLayout;

	QPushButton *bcButton = new QPushButton("->");
	l->addWidget(bcButton, 0, 0);
	l->addWidget(new QLabel("BEGIN CONQUEST"), 0, 1);
	QObject::connect(bcButton, &QPushButton::clicked, [this]() { result = "BEGIN CONQUEST\n"; done(0); });

	QPushButton *cButton = new QPushButton("->");
	l->addWidget(cButton, 1, 0);
	l->addWidget(new QLabel("CONQUEST"), 1, 1);
	QLineEdit *cX1 = new QLineEdit, *cY1 = new QLineEdit, *cX2 = new QLineEdit, *cY2 = new QLineEdit;
	edits << cX1 << cY1 << cX2 << cY2;
	l->addWidget(cX1, 1, 2);
	l->addWidget(cY1, 1, 3);
	l->addWidget(cX2, 1, 4);
	l->addWidget(cY2, 1, 5);
	QObject::connect(cButton, &QPushButton::clicked, [this, cX1, cY1, cX2, cY2]() {
		result = QString("CONQUEST %1 %2 %3 %4\n").arg(cX1->text(), cY1->text(), cX2->text(), cY2->text());
		done(0);
	});
	
	QPushButton *bfButton = new QPushButton("->");
	l->addWidget(bfButton, 2, 0);
	l->addWidget(new QLabel("BEGIN FORTIFICATION"), 2, 1);
	QObject::connect(bfButton, &QPushButton::clicked, [this]() { result = "BEGIN FORTIFICATION\n"; done(0); });


	QPushButton *fButton = new QPushButton("->");
	l->addWidget(fButton, 3, 0);
	l->addWidget(new QLabel("FORTIFY"), 3, 1);
	QLineEdit *fX = new QLineEdit, *fY = new QLineEdit;
	edits << fX << fY;
	l->addWidget(fX, 3, 2);
	l->addWidget(fY, 3, 3);
	QObject::connect(fButton, &QPushButton::clicked, [this, fX, fY]() {
		result = QString("FORTIFY %1 %2\n").arg(fX->text(), fY->text());
		done(0);
	});

	QPushButton *pButton = new QPushButton("->");
	l->addWidget(pButton, 4, 0);
	l->addWidget(new QLabel("PASS"), 4, 1);
	QObject::connect(pButton, &QPushButton::clicked, [this]() { result = "PASS\n"; done(0); });


	QPushButton *dButton = new QPushButton("->");
	l->addWidget(dButton, 5, 0);
	l->addWidget(new QLabel("ДИСКВАЛИФИЦИРОВАТЬСЯ"), 5, 1);
	QObject::connect(dButton, &QPushButton::clicked, [this]() {
		result = ""; 
		if (QMessageBox::question(this, "Confirmation", "Вы точно хотите ДИСКВАЛИФИЦИРОВАТЬСЯ?",
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		{
			done(0);
		}
	});

	for (auto i : edits)
	{
		i->setStyleSheet("border: 1px solid gray; width: 25px; height:25px;");
	}

	setLayout(l);
}

void HumanPlayer::write(const std::string& s)
{
	std::stringstream ss(s);
	std::string cmd;
	ss >> cmd;
	int id;
	if (cmd == "HELLO")
	{
		ss >> id;
		setWindowTitle("Ход игрока " + QString::number(id));
	}

}

std::string HumanPlayer::read()
{
	for (auto i : edits)
	{
		i->setText("");
	}

	exec();

	return result.toStdString();
}

GameUi::GameUi(const QString& map, const QStringList& players)
{
	game = new Game;
	for (auto &i : players)
	{
		if (i != "__human__")
		{
			QProcess *p = new QProcess(this);
			p->start(i);
			if (!p->waitForStarted())
			{
				QMessageBox::critical(this, "Error starting program", p->errorString());
			}
			game->addPlayer(new QIODevicePlayer(p));
		}
		else
		{
			game->addPlayer(new HumanPlayer()); // форма
		}
	}
	log = new QPlainTextEdit;
	game->loadMap(map.toStdString());
	QSplitter *l = new QSplitter;
	QScrollArea *area = new QScrollArea;
	l->addWidget(area);
	
	field = new FieldWidget(game->getCells());
	area->setWidget(field);

	l->addWidget(log);
	QPushButton *step = new QPushButton("Step");
	QObject::connect(step, &QPushButton::clicked, [this]() { game->step(); });
	QVBoxLayout *l2 = new QVBoxLayout;
	l2->addWidget(l);
	l2->addWidget(step);
	setLayout(l2);
	Log::setLogWriter(this);
}

void GameUi::addComment(const std::string& s)
{
	log->appendPlainText(QString::fromUtf8(s.c_str()));
	QApplication::processEvents();
}

void GameUi::addCell(Cell*)
{
	field->update();
}

FieldWidget::FieldWidget(const std::vector<Cell*>& _cells):
	cells(_cells)
{
	hexagon.moveTo(hexagonCoord(0));
	for (int i = 1; i <= 6; ++i)
	{
		hexagon.lineTo(hexagonCoord(i));
	}

	int mx = 0, my = 0;
	for (auto i : cells)
	{
		auto c = centerCoord(i->coord);
		mx = qMax(mx, c.x());
		my = qMax(my, c.y());
	}
	sizeHintCache = QSize(mx + 3 * r, my + 3 * r);

}

QPoint FieldWidget::hexagonCoord(int i)
{
	i %= 6;
	double pi3 = M_PI / 3;
	return QPoint(r * cos(pi3 * i), r * sin(pi3 * i));
}

QPoint FieldWidget::centerCoord(int i, int j)
{
	++i, ++j;
	return {r * 3 * j + (3 * r * (i % 2 == 0) / 2), r * 3 * i};
}

QPoint FieldWidget::centerCoord(Coord c)
{
	return centerCoord(c.x, c.y);
}

QColor FieldWidget::colors[5] = { Qt::gray, "cyan", "magenta", "gold", "red"},
	FieldWidget::textColors[5] = { Qt::black, Qt::black, Qt::black, Qt::black, Qt::white };

void FieldWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);
	painter.fillRect(0, 0, sizeHint().width(), sizeHint().height(), Qt::black);
	QFont small, big;
	small.setPointSize(10);
	big.setBold(true);
	for (auto i : cells)
	{
		painter.setPen(QColor(Qt::white));
		for (auto j : i->neigh)
		{
			if (i->coord < j->coord)
				painter.drawLine(centerCoord(i->coord), centerCoord(j->coord));
		}
		painter.translate(centerCoord(i->coord));
		int owner = i->owner ? i->owner->id() : 0;
		QRect coordRect;
		auto c4 = hexagonCoord(4), c5 = hexagonCoord(5);
		c4.ry() -= 12;
		c5.ry() -= 2;
		painter.setFont(small);
		painter.drawText(QRect(c4, c5), Qt::AlignCenter, QString("(%1,%2)").arg(i->coord.x).arg(i->coord.y));
		painter.fillPath(hexagon, colors[owner]);
		painter.setPen(textColors[owner]);
		painter.setFont(big);
		auto c3 = hexagonCoord(3), c6 = hexagonCoord(6);
		c3.ry() -= 6;
		c6.ry() += 6;
		painter.drawText(QRect(c3, c6), Qt::AlignCenter, QString("%1 / %2").arg(i->power).arg(i->size));
		painter.resetTransform();
	}
}

QSize FieldWidget::sizeHint() const
{
	return sizeHintCache;
}
