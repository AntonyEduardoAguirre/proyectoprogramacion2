#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_registro.h"
#include <iostream>
#include <string>
#include <vector>



using namespace std;

class registro : public QMainWindow
{
    Q_OBJECT

public:
    registro(QWidget* parent = nullptr);
    ~registro();

private:
	void crearUsuario();
    Ui::registroClass ui;
};