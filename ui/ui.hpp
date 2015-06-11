#ifndef UI_HPP
#define UI_HPP

#include <QtCore>
#include <QtWidgets>
#include "manager.hpp"

class StartupUi;

class StartupUiPlayer : public QComboBox
{
	Q_OBJECT
	public:
		int id;
		StartupUi *ui;
		QString path;
		StartupUiPlayer(int _id, StartupUi* _ui);

	public slots:
		void idxChanged(int idx);
};

class StartupUi : public QWidget
{
	Q_OBJECT
	public:
		StartupUi();
	private slots:
		void loadMap();
		void start();	
	private:
		QList<StartupUiPlayer*> players;
		QString map;

	friend class StartupUiPlayer;
};

class FieldWidget : public QWidget
{
	Q_OBJECT
	public:
		FieldWidget(const std::vector<Cell*>& _cells);
	protected:
		void paintEvent(QPaintEvent* e);
		QSize sizeHint() const;
	private:
		QPoint hexagonCoord(int i);
		QPoint centerCoord(int i, int j);
		QPoint centerCoord(Coord c);
		void drawHexagon(QPainter *painter, QPoint center);
		int r = 26;
		QPainterPath hexagon;
		static QColor colors[5], textColors[5];
		QSize sizeHintCache;
		std::vector<Cell*> cells;
};

class GameUi : public QWidget, public LogWriter
{
	Q_OBJECT
	public:
		GameUi(const QString& _map, const QStringList& _players);
		Game* game;
		QPlainTextEdit *log;
		FieldWidget *field;
		void addComment(const std::string& s);
		void addCell(Cell* c);
};

class HumanPlayer : public QDialog, public TextPlayer
{
	Q_OBJECT
	public:
		int id;
		HumanPlayer();
		void write(const std::string&);
		std::string read();
		QString result;
		QList<QLineEdit*> edits;
}; 

#endif
