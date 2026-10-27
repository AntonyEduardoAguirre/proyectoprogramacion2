#pragma once

#include <QWidget>
#include "ui_seguro.h"

class seguro : public QWidget
{
    Q_OBJECT

public:
    seguro(QWidget* parent = nullptr);
    ~seguro();

private:
    void desactivarCuenta();
    Ui::SeguroClass ui;
};