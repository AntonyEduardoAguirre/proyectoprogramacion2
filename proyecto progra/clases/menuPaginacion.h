#pragma once
#include <QList>
#include <QMainWindow>

#include "datos.h"

class QFrame;
class QLabel;
class QPushButton;

namespace Ui { class MenuPaginacion; }

// MENÚ DE PAGINACIÓN (ventana de catálogo).
class MenuPaginacion : public QMainWindow
{
    Q_OBJECT

public:
    explicit MenuPaginacion(QWidget* parent = nullptr);
    ~MenuPaginacion() override;

    void actualizarCatalogo();   // recarga productos y rehace la búsqueda (si el catálogo cambia)

private slots:
    void aplicarFiltros();
    void agregarPorCodigo();
    void paginaAnterior();
    void paginaSiguiente();
    void abrirDetalle(int indiceProducto);
    void volverAlMenu();
    void comprarActual();        // agrega el producto del detalle al carrito
    void mostrarCarrito();       // abre la ventana del carrito

private:
    void mostrarPagina();
    void actualizarPanelCarrito();   // refresca el panel de productos acumulados

    Ui::MenuPaginacion* ui;

    bool admin = false;                 // true => también muestra productos con stock 0
    static constexpr int PRODUCTOS_POR_PAGINA = 9;

    QList<int> filtrados;               // índices de "productos" que pasan la búsqueda por nombre
    QList<BotonProducto> botones;       // los 9 botones de la página
    int paginaActual = 0;
    int detalleIndice = -1;             // producto mostrado en la pantalla de detalle
};
