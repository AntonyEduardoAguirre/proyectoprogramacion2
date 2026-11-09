#pragma once
#include <QList>
#include <QMainWindow>

#include "datos.h"

class QPushButton;

namespace Ui { class MenuPaginacion; }

// MENÚ DE PAGINACIÓN (ventana de catálogo).
// muestra 6 productos por página y abre una pantalla de detalle al elegir uno.
class MenuPaginacion : public QMainWindow
{
    Q_OBJECT

public:
    explicit MenuPaginacion(QWidget* parent = nullptr);
    ~MenuPaginacion() override;

    void actualizarCatalogo();   // recarga productos y rehace filtros (si el catálogo cambia)

private slots:
    void aplicarFiltros();
    void paginaAnterior();
    void paginaSiguiente();
    void abrirDetalle(int indiceProducto);
    void volverAlMenu();
    void comprarActual();        // agrega el producto del detalle al carrito
    void mostrarCarrito();       // abre la ventana del carrito

private:
    void configurarSlider();
    void mostrarPagina();
    void actualizarBotonCarrito();

    Ui::MenuPaginacion* ui;

    bool admin = false;                 // true => también muestra productos con stock 0
    static constexpr int PRODUCTOS_POR_PAGINA = 6;

    QList<int> filtrados;               // índices de "productos" que pasan los filtros
    QList<QPushButton*> botones;        // los 6 botones de la página
    int paginaActual = 0;
    int detalleIndice = -1;             // producto mostrado en la pantalla de detalle
};
