#pragma once

#include <QWidget>
#include "ui_carrito.h"
#include "compra.h"

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
    void abrirPagoEfectivo();
    void abrirPagoTarjeta(int meses);
    void abrirPagoCuotas(int mesesIniciales);
    void mostrarRecibo(const Compra& compra, double efectivoRecibido, double vuelto);
    void actualizarCarrito();
    void mostrarTotal();
    Ui::carritoClass ui;
};
