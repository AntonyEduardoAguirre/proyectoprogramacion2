#include "clases/menuPaginacion.h"
#include "ui_menuPaginacion.h"  // generado por uic a partir de menuPaginacion.ui
#include "clases/datos.h"
#include "Carrito.h"

#include <algorithm>

#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QVBoxLayout>

MenuPaginacion::MenuPaginacion(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MenuPaginacion)
{
    ui->setupUi(this);
	// La sombra suave no se puede definir en el .ui
    for (QFrame* tarjeta : { ui->cardDetalle }) {
        auto* sombra = new QGraphicsDropShadowEffect(tarjeta);
        sombra->setBlurRadius(28);
        sombra->setOffset(0, 8);
        sombra->setColor(QColor(41, 42, 40, 22));
        tarjeta->setGraphicsEffect(sombra);
    }

    // --- 6 botones de producto en la cuadrícula (2 filas x 3 columnas) ---
    for (int i = 0; i < PRODUCTOS_POR_PAGINA; ++i) {
        auto* b = new QPushButton(this);
        b->setObjectName("btnProducto");
        b->setMinimumSize(160, 90);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->gridProductos->addWidget(b, i / 3, i % 3);
        botones.append(b);

        connect(b, &QPushButton::clicked, this, [this, i] {
            const int pos = paginaActual * PRODUCTOS_POR_PAGINA + i;
            if (pos < filtrados.size())
                abrirDetalle(filtrados[pos]);
        });
    }

    // --- Búsqueda por nombre e ID (cada campo se activa con su checkbox) ---
    ui->leNombre->setEnabled(false);
    ui->leId->setEnabled(false);

    connect(ui->chkNombre, &QCheckBox::toggled, ui->leNombre, &QWidget::setEnabled);
    connect(ui->chkId,     &QCheckBox::toggled, ui->leId,     &QWidget::setEnabled);

    connect(ui->chkNombre, &QCheckBox::toggled, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->chkId,     &QCheckBox::toggled, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->leNombre,  &QLineEdit::textChanged, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->leId,      &QLineEdit::textChanged, this, &MenuPaginacion::aplicarFiltros);

    // --- Paginación ---
    connect(ui->btnAnterior,  &QPushButton::clicked, this, &MenuPaginacion::paginaAnterior);
    connect(ui->btnSiguiente, &QPushButton::clicked, this, &MenuPaginacion::paginaSiguiente);

    // --- Detalle ---
    connect(ui->btnSalir, &QPushButton::clicked, this, &MenuPaginacion::volverAlMenu);
    connect(ui->btnComprar, &QPushButton::clicked, this, &MenuPaginacion::comprarActual);

    // --- Carrito (misma acción/ventana que antes) ---
    connect(ui->btnCarrito, &QPushButton::clicked, this, &MenuPaginacion::mostrarCarrito);

    // --- Regresar: cierra el catálogo y MenuPrincipal vuelve a mostrarse ---
    connect(ui->btnRegresar, &QPushButton::clicked, this, &QWidget::close);

    actualizarCatalogo();
    actualizarPanelCarrito();
}


MenuPaginacion::~MenuPaginacion()
{
    delete ui;
}

void MenuPaginacion::actualizarCatalogo()
{
    if (::productos.isEmpty())
        cargarProductos();      // carga productos.json o lo crea con el catálogo inicial

    aplicarFiltros();
    actualizarPanelCarrito();
}

// Los filtros activos se combinan con AND (búsqueda por nombre e ID).
void MenuPaginacion::aplicarFiltros()
{
    filtrados.clear();

    const QString nombre = ui->leNombre->text().trimmed();
    const QString idTxt  = ui->leId->text().trimmed();

    for (int i = 0; i < productos.size(); ++i) {
        const Producto& p = productos[i];

        if (!admin && p.stock <= 0) continue;   // regla de stock

        if (ui->chkNombre->isChecked() && !nombre.isEmpty()
            && !p.nombre.contains(nombre, Qt::CaseInsensitive)) continue;

        if (ui->chkId->isChecked() && !idTxt.isEmpty()
            && p.id != idTxt.toInt()) continue;

        filtrados.append(i);
    }

    paginaActual = 0;    // al cambiar el filtro, volvemos a la primera página
    mostrarPagina();
}

void MenuPaginacion::mostrarPagina()
{
    int totalPaginas = (filtrados.size() + PRODUCTOS_POR_PAGINA - 1) / PRODUCTOS_POR_PAGINA;
    if (totalPaginas == 0) totalPaginas = 1;

    paginaActual = qBound(0, paginaActual, totalPaginas - 1);

    for (int i = 0; i < PRODUCTOS_POR_PAGINA; ++i) {
        const int pos = paginaActual * PRODUCTOS_POR_PAGINA + i;
        QPushButton* b = botones[i];

        if (pos < filtrados.size()) {
            const Producto& p = productos[filtrados[pos]];
            b->setText(QString("%1\n$%2").arg(p.nombre).arg(p.precio, 0, 'f', 2));
            b->setVisible(true);
        } else {
            b->setVisible(false);   // hueco vacío en la última página
        }
    }

    ui->lblPagina->setText(tr("Página %1 de %2").arg(paginaActual + 1).arg(totalPaginas));
    ui->btnAnterior->setEnabled(paginaActual > 0);
    ui->btnSiguiente->setEnabled(paginaActual < totalPaginas - 1);
}

void MenuPaginacion::paginaAnterior()  { --paginaActual; mostrarPagina(); }
void MenuPaginacion::paginaSiguiente() { ++paginaActual; mostrarPagina(); }

void MenuPaginacion::abrirDetalle(int indiceProducto)
{
    detalleIndice = indiceProducto;
    const Producto& p = productos[indiceProducto];

    ui->lblDetNombre->setText(tr("Nombre: %1").arg(p.nombre));
    ui->lblDetCategoria->setText(tr("Categoría: %1").arg(p.categoria));
    ui->lblDetPrecio->setText(tr("Precio: $%1").arg(p.precio, 0, 'f', 2));
    ui->lblDetId->setText(tr("ID: %1").arg(p.id));
    ui->lblDetStock->setText(tr("Stock: %1").arg(p.stock));

    ui->stackedWidget->setCurrentIndex(1);   // muestra el detalle
}

void MenuPaginacion::volverAlMenu()
{
    ui->stackedWidget->setCurrentIndex(0);   // Comprar: por ahora solo regresa al menú
}

// Agrega el producto del detalle al carrito (1 unidad)
void MenuPaginacion::comprarActual()
{
    if (detalleIndice < 0 || detalleIndice >= productos.size())
        return;

    const Producto& p = productos[detalleIndice];

    if (!agregarAlCarrito(p.id, 1)) {
        QMessageBox::warning(this, "Carrito",
            "No hay más stock disponible de este producto.");
        return;
    }

    volverAlMenu();            // regresa al catálogo
    actualizarPanelCarrito();  // muestra el producto y recalcula el total al instante
}

// Abre la ventana del carrito sobre el catálogo (misma acción y ventana que antes)
void MenuPaginacion::mostrarCarrito()
{
    Carrito* ventanaCarrito = new Carrito();
    ventanaCarrito->setAttribute(Qt::WA_DeleteOnClose);
    connect(ventanaCarrito, &Carrito::carritoModificado, this, [this]() {
        actualizarCatalogo();      // refresca stock/filtros y el panel tras pagar/quitar
        actualizarPanelCarrito();
        });
    ventanaCarrito->setWindowModality(Qt::ApplicationModal);
    ventanaCarrito->show();
}

// Refresca el panel izquierdo leyendo los mismos datos compartidos del carrito (datos.h)
void MenuPaginacion::actualizarPanelCarrito()
{
    // limpia los elementos anteriores del panel
    while (QLayoutItem* hijo = ui->layoutCarritoPanel->takeAt(0)) {
        if (QWidget* widget = hijo->widget())
            widget->deleteLater();
        delete hijo;
    }

    if (carrito.isEmpty()) {
        auto* vacio = new QLabel(tr("Tu carrito está vacío."), this);
        vacio->setObjectName("lblCarritoVacio");
        vacio->setAlignment(Qt::AlignCenter);
        vacio->setWordWrap(true);
        ui->layoutCarritoPanel->addWidget(vacio);
    } else {
        for (const ItemCompra& item : carrito) {
            auto* fila = new QLabel(
                QString("%1 × %2\n$%3 c/u  —  $%4")
                    .arg(item.cantidad)
                    .arg(item.nombre)
                    .arg(item.precio, 0, 'f', 2)
                    .arg(item.precio * item.cantidad, 0, 'f', 2),
                this);
            fila->setObjectName("lblItemCarritoPanel");
            fila->setWordWrap(true);
            ui->layoutCarritoPanel->addWidget(fila);
        }
    }
    ui->layoutCarritoPanel->addStretch();

    ui->lblTotalPanel->setText(QString("$%1").arg(carritoTotal(), 0, 'f', 2));
}
