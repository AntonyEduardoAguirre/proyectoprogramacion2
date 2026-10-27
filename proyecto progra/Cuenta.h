#pragma once

#include <QWidget>
#include "ui_cuenta.h"

class cuenta : public QWidget
{
    Q_OBJECT

public:
    cuenta(QWidget* parent = nullptr);
    ~cuenta();

private:
    void actualizarDatos();
    Ui::cuentaClass ui;
};