#ifndef SYSTESTS_H
#define SYSTESTS_H

#include <QObject>

class SysTests: public QObject {
    Q_OBJECT

public:
    SysTests(QObject *parent = nullptr) : QObject(parent) {;}

private Q_SLOTS:
    void findFullPathsTest();
    void findProcessIdTest();
};

#endif // SYSTESTS_H
