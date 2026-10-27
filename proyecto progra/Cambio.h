#pragma once

#include <QWidget>
#include "ui_cambio.h"

class cambio : public QWidget
{
    Q_OBJECT

public:
    cambio(QWidget* parent = nullptr);
    ~cambio();

private:
	void cambiarDatos(); // Función para cambiar los datos del usuario
    Ui::CambioClass ui;
};