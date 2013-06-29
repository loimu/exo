#ifndef EXO_H
#define EXO_H

#include <QApplication>

class Exo : public QApplication
{
    Q_OBJECT

public:
    explicit Exo(int &argc, char **argv);

signals:

private slots:

};

#endif // EXO_H
