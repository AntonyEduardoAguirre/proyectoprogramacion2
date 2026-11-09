#pragma once

#include <QWidget>
#include "ui_carrito.h"

// La clase se llama "Carrito" (mayúscula) para no chocar con la
// variable global "carrito" declarada en datos.h
class Carrito : public QWidget
{
    Q_OBJECT

public:
    Carrito(QWidget* parent = nullptr);
    ~Carrito();

signals:
    void carritoModificado();   // avisa al catálogo para refrescar el botón/stock

private:
    void actualizarCarrito();
    Ui::carritoClass ui;
};
