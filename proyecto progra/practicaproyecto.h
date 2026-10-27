#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_practicaproyecto.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class practicaproyecto : public QMainWindow
{
    Q_OBJECT

public:
    practicaproyecto(QWidget *parent = nullptr);
    ~practicaproyecto();

private:
    bool validarSesion();
    Ui::practicaproyectoClass ui;
};




