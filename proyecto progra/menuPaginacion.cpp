#include "menuPaginacion.h"
#include "ui_menuPaginacion.h"  // generado por uic a partir de menuPaginacion.ui
#include "datos.h"            

#include <algorithm>

#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QtMath>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

MenuPaginacion::MenuPaginacion(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MenuPaginacion)
{
    ui->setupUi(this);
	// La sombra suave no se puede definir en el .ui
    for (QFrame* tarjeta : { ui->cardFiltros, ui->cardCatalogo, ui->cardDetalle }) {
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

    // --- Cada campo de filtro solo se activa si su checkbox está marcado ---
    ui->leNombre->setEnabled(false);
    ui->leCategoria->setEnabled(false);
    ui->leId->setEnabled(false);
    ui->sldPrecio->setEnabled(false);

    connect(ui->chkNombre,    &QCheckBox::toggled, ui->leNombre,    &QWidget::setEnabled);
    connect(ui->chkCategoria, &QCheckBox::toggled, ui->leCategoria, &QWidget::setEnabled);
    connect(ui->chkId,        &QCheckBox::toggled, ui->leId,        &QWidget::setEnabled);
    connect(ui->chkPrecio,    &QCheckBox::toggled, ui->sldPrecio,   &QWidget::setEnabled);

    // --- Cualquier cambio en filtros => recalcular ---
    connect(ui->chkNombre,    &QCheckBox::toggled, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->chkCategoria, &QCheckBox::toggled, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->chkId,        &QCheckBox::toggled, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->chkPrecio,    &QCheckBox::toggled, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->leNombre,     &QLineEdit::textChanged, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->leCategoria,  &QLineEdit::textChanged, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->leId,         &QLineEdit::textChanged, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->sldPrecio,    &QSlider::valueChanged, this, [this](int v) {
        ui->lblPrecio->setText(QString("$%1").arg(v));
        aplicarFiltros();
    });

    // --- Paginación ---
    connect(ui->btnAnterior,  &QPushButton::clicked, this, &MenuPaginacion::paginaAnterior);
    connect(ui->btnSiguiente, &QPushButton::clicked, this, &MenuPaginacion::paginaSiguiente);

    // --- Detalle ---
    connect(ui->btnSalir, &QPushButton::clicked, this, &MenuPaginacion::volverAlMenu);
    // btnComprar: sin conexión por ahora 

    actualizarCatalogo();
}


MenuPaginacion::~MenuPaginacion()
{
    delete ui;
}

void MenuPaginacion::actualizarCatalogo()
{
    if (::productos.isEmpty())
        cargarProductos();      // carga productos.json o lo crea con el catálogo inicial

    configurarSlider();
    aplicarFiltros();
}

// El rango del slider sale de los productos reales, así que se adapta si el catálogo cambia.
void MenuPaginacion::configurarSlider()
{
    double minP = 0, maxP = 0;
    bool primero = true;

    for (const Producto& p : productos) {
        if (!admin && p.stock <= 0) continue;
        if (primero) { minP = maxP = p.precio; primero = false; }
        minP = std::min(minP, p.precio);
        maxP = std::max(maxP, p.precio);
    }

    const QSignalBlocker bloqueo(ui->sldPrecio);   // evita disparar el filtro aquí
    ui->sldPrecio->setRange(static_cast<int>(qFloor(minP)), static_cast<int>(qCeil(maxP)));
    ui->sldPrecio->setValue(static_cast<int>(qCeil(maxP)));   // por defecto: sin recortar nada
    ui->lblPrecio->setText(QString("$%1").arg(ui->sldPrecio->value()));
}

// Los filtros activos se combinan con AND.
void MenuPaginacion::aplicarFiltros()
{
    filtrados.clear();

    const QString nombre    = ui->leNombre->text().trimmed();
    const QString categoria = ui->leCategoria->text().trimmed();
    const QString idTxt     = ui->leId->text().trimmed();
    const int     precioMax = ui->sldPrecio->value();

    for (int i = 0; i < productos.size(); ++i) {
        const Producto& p = productos[i];

        if (!admin && p.stock <= 0) continue;   // regla de stock

        if (ui->chkNombre->isChecked() && !nombre.isEmpty()
            && !p.nombre.contains(nombre, Qt::CaseInsensitive)) continue;

        if (ui->chkCategoria->isChecked() && !categoria.isEmpty()
            && !p.categoria.contains(categoria, Qt::CaseInsensitive)) continue;

        if (ui->chkId->isChecked() && !idTxt.isEmpty()
            && p.id != idTxt.toInt()) continue;

        if (ui->chkPrecio->isChecked() && p.precio > precioMax) continue;

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
