#pragma once

#include <QWidget>
#include "ui_saldo.h"

class Saldo : public QWidget
{
    Q_OBJECT

public:
    Saldo(QWidget* parent = nullptr);
    ~Saldo();

private:
    void actualizarDatos();
    Ui::saldoClass ui;
};
