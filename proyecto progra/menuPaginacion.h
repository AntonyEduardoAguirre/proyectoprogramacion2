#pragma once
#include <QList>
#include <QMainWindow>

#include "datos.h"

class QPushButton;

namespace Ui { class MenuPaginacion; }

// MENÚ DE PAGINACIÓN (ventana de catálogo). Su diseño está en menuPaginacion.ui.
// Filtra productos por nombre, categoría, ID y precio máximo (los filtros se combinan con AND),
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

private:
    void configurarSlider();
    void mostrarPagina();

    Ui::MenuPaginacion* ui;

    bool admin = false;                 // true => también muestra productos con stock 0
    static constexpr int PRODUCTOS_POR_PAGINA = 6;

    QList<Producto> productos;          // catálogo cargado desde datos.h
    QList<int> filtrados;               // índices de "productos" que pasan los filtros
    QList<QPushButton*> botones;        // los 6 botones de la página
    int paginaActual = 0;
};
