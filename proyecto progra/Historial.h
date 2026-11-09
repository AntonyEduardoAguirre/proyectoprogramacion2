#pragma once

#include <QWidget>
#include "ui_historial.h"

class historial : public QWidget
{
    Q_OBJECT

public:
    historial(QWidget* parent = nullptr);
    ~historial();

private:
    void actualizarHistorial();
    Ui::historialClass ui;
};
