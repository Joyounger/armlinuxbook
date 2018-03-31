#ifndef __eightapplet_h__
#define __eightapplet_h__

#include <QMainWindow>
#include <QAbstractTableModel>
#include <QTableView>
#include <QAbstractItemDelegate>
#include <QList>
#include <QPolygon>

class QMenu;

class PiecesTable : public QAbstractTableModel
{
    Q_OBJECT

public:
    PiecesTable(QObject* parent = 0);
    ~PiecesTable();

    int rowCount(const QModelIndex & = QModelIndex()) const { return 3; }
    int columnCount(const QModelIndex & = QModelIndex()) const { return 3; }

    QVariant data(const QModelIndex &, int role = Qt::DisplayRole) const;

    void push(const QModelIndex &index)
    { push(index.column(), index.row()); }
    void push(const QPoint &index)
    { push(index.x(), index.y()); }

    void pushLeft();
    void pushRight();
    void pushUp();
    void pushDown();
    bool useImage() { return !_imgName.isEmpty(); }

public slots:
    void randomize();
    void reset();
    void showNumber();
    void loadImage();
    void deleteImage();

signals:
    void gameWon();
    void updateMenu(bool);

protected:
    void initMap();
    void initColors();
    void readConfig();
    void writeConfig();
    void checkwin();
    void sliceImage();

private:
    QPoint findPoint(int);
    void push(int col, int row);
    QList<int>    _map;
    QList<QColor> _colors;
    QList<QImage> _images;
    bool _randomized;
    QString _imgName;
};

class PiecesView : public QTableView
{
    Q_OBJECT

public:
    PiecesView(QWidget *parent);

    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent* e);
    void mousePressEvent(QMouseEvent* e);

    void setModel(QAbstractItemModel *);

public slots:
    void announceWin();

private:
    QMenu     *_menu;
    QAction *randomizeAction;
    QAction *resetAction;
    bool rtl;
};

class PiecesDelegate : public QAbstractItemDelegate
{
public:
    PiecesDelegate(QWidget *parent) : QAbstractItemDelegate(parent) {}

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

    QSize sizeHint ( const QStyleOptionViewItem &, const QModelIndex & ) const { return cellSize; }

    static QSize cellSize;
    static QPolygon light_border;
    static QPolygon dark_border;
};

class EightMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    EightMainWindow(QWidget *parent=0, Qt::WFlags fl=0);

public slots:
    void showNumber();
    void updateMenu(bool);

private:
    QAction *actionShowNum, *actionLoadImg, *actionDeleteImg;
};

#endif
