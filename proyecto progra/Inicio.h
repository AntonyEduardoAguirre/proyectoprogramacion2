#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_inicio.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class inicio : public QMainWindow
{
    Q_OBJECT

public:
    inicio(QWidget *parent = nullptr);
    ~inicio();

private:
    void validarSesion();
    Ui::inicioClass ui;
};




