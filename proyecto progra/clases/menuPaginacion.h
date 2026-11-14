#pragma once
#pragma once
#include <QList>
#include <QMainWindow>

#include "datos.h"

class QFrame;
class QLabel;
class QPushButton;

namespace Ui { class MenuPaginacion; }

// Una línea del carrito: el producto se identifica por su código de 6 dígitos.
struct ItemCarrito
{
    QString codigo;
    int cantidad = 0;
};

// MENÚ DE PAGINACIÓN (ventana de catálogo + carrito).
// Muestra 6 productos por página, con dos buscadores (nombre y código).
// Al hacer clic en un producto se agrega al carrito.
class MenuPaginacion : public QMainWindow
{
    Q_OBJECT

public:
    explicit MenuPaginacion(QWidget* parent = nullptr);
    ~MenuPaginacion() override;

    void actualizarCatalogo();   // recarga productos y rehace la búsqueda (si el catálogo cambia)

private slots:
    void aplicarFiltros();
    void paginaAnterior();
    void paginaSiguiente();
    void agregarAlCarrito(int indiceProducto);
    void vaciarCarrito();
    void pagar();

private:
    void mostrarPagina();
    void reconstruirCarrito();
    void cambiarCantidad(const QString& codigo, int delta);
    void quitarDelCarrito(const QString& codigo);
    int  indiceDeCodigo(const QString& codigo) const;   // posición en "productos" o -1

    // Los 6 botones de producto y las etiquetas que llevan dentro
    struct BotonProducto
    {
        QPushButton* boton;
        QLabel* nombre;
        QLabel* precio;
        QLabel* codigo;
    };

    Ui::MenuPaginacion* ui;

    bool admin = false;                 // true => también muestra productos con stock 0
    static constexpr int PRODUCTOS_POR_PAGINA = 6;

    QList<int> filtrados;               // índices de "productos" que pasan la búsqueda
    QList<BotonProducto> botones;       // los 6 botones de la página
    int paginaActual = 0;

    QList<ItemCarrito> carrito;
    QList<QFrame*> filasCarrito;        // widgets de las filas actuales del carrito
};