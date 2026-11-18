#include "clases/menuPaginacion.h"
#include "ui_menuPaginacion.h"  // generado por uic a partir de menuPaginacion.ui
#include "clases/datos.h"
#include "Carrito.h"          // ventana que cobra (efectivo, tarjeta y cuotas)

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

MenuPaginacion::MenuPaginacion(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MenuPaginacion)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);

    // La cuadrícula de productos ocupa todo el espacio libre de la tarjeta
    ui->layoutCatalogo->setStretchFactor(ui->gridProductos, 1);

    // --- 9 botones de producto en la cuadrícula (3 filas x 3 columnas) ---
    // Cada botón lleva tres etiquetas (nombre, precio, código) para poder
    // ajustar el texto largo y dar al código un color más suave.
    for (int i = 0; i < PRODUCTOS_POR_PAGINA; ++i) {
        auto* b = new QPushButton(this);
        b->setObjectName("btnProducto");
        b->setMinimumSize(150, 90);
        b->setCursor(Qt::PointingHandCursor);

        QSizePolicy politica(QSizePolicy::Expanding, QSizePolicy::Expanding);
        politica.setRetainSizeWhenHidden(true);   // el hueco vacío conserva su tamaño
        b->setSizePolicy(politica);

        auto* capa = new QVBoxLayout(b);
        capa->setContentsMargins(8, 8, 8, 8);
        capa->setSpacing(2);

        auto crearEtiqueta = [b, capa](const char* objeto, bool ajustar) {
            auto* l = new QLabel(b);
            l->setObjectName(objeto);
            l->setAlignment(Qt::AlignCenter);
            l->setWordWrap(ajustar);
            l->setAttribute(Qt::WA_TransparentForMouseEvents);   // el clic llega al botón
            capa->addWidget(l);
            return l;
            };

        capa->addStretch();
        QLabel* nombre = crearEtiqueta("prodNombre", true);
        QLabel* precio = crearEtiqueta("prodPrecio", false);
        QLabel* codigo = crearEtiqueta("prodCodigo", false);
        capa->addStretch();

        ui->gridProductos->addWidget(b, i / 3, i % 3);
        botones.append(BotonProducto{ b, nombre, precio, codigo });

        connect(b, &QPushButton::clicked, this, [this, i] {
            const int pos = paginaActual * PRODUCTOS_POR_PAGINA + i;
            if (pos < filtrados.size())
                agregarAlCarrito(filtrados[pos]);
            });
    }

    // --- Buscadores ---
    // Nombre: filtra el catálogo mientras se escribe.
    // Código: NO filtra; al presionar Enter agrega al carrito el producto con ese código.
    // El código admite al menos 3 dígitos y puede comenzar con cero.
    ui->leCodigo->setValidator(
        new QRegularExpressionValidator(QRegularExpression("\\d*"), this));

    connect(ui->leNombre, &QLineEdit::textChanged, this, &MenuPaginacion::aplicarFiltros);
    connect(ui->leCodigo, &QLineEdit::returnPressed, this, &MenuPaginacion::agregarPorCodigo);

    // --- Paginación ---
    connect(ui->btnAnterior, &QPushButton::clicked, this, &MenuPaginacion::paginaAnterior);
    connect(ui->btnSiguiente, &QPushButton::clicked, this, &MenuPaginacion::paginaSiguiente);

    // --- Carrito ---
    connect(ui->btnPagar, &QPushButton::clicked, this, &MenuPaginacion::pagar);

    actualizarCatalogo();
}


MenuPaginacion::~MenuPaginacion()
{
    delete ui;
}

void MenuPaginacion::actualizarCatalogo()
{
    if (::productos.isEmpty())
        cargarProductos();      // carga productos.json

    asegurarCodigos();          // todo producto debe tener su código de 6 dígitos

    aplicarFiltros();
    reconstruirCarrito();       // quita del carrito lo que ya no exista o no tenga stock
}

int MenuPaginacion::indiceDeId(int idProducto) const
{
    for (int i = 0; i < productos.size(); ++i)
        if (productos[i].id == idProducto)
            return i;
    return -1;
}

int MenuPaginacion::indiceDeCodigo(const QString& codigo) const
{
    for (int i = 0; i < productos.size(); ++i)
        if (productos[i].codigo == codigo)
            return i;
    return -1;
}

// Solo el buscador por nombre filtra el catálogo (si está vacío no filtra nada).
// El buscador por código se maneja aparte, en agregarPorCodigo().
void MenuPaginacion::aplicarFiltros()
{
    filtrados.clear();

    const QString nombre = ui->leNombre->text().trimmed();

    for (int i = 0; i < productos.size(); ++i) {
        const Producto& p = productos[i];

        if (!admin && p.stock <= 0) continue;   // regla de stock

        if (!nombre.isEmpty() && !p.nombre.contains(nombre, Qt::CaseInsensitive)) continue;

        filtrados.append(i);
    }

    paginaActual = 0;    // al cambiar la búsqueda, volvemos a la primera página
    mostrarPagina();
}

void MenuPaginacion::mostrarPagina()
{
    int totalPaginas = (filtrados.size() + PRODUCTOS_POR_PAGINA - 1) / PRODUCTOS_POR_PAGINA;
    if (totalPaginas == 0) totalPaginas = 1;

    paginaActual = qBound(0, paginaActual, totalPaginas - 1);

    for (int i = 0; i < PRODUCTOS_POR_PAGINA; ++i) {
        const int pos = paginaActual * PRODUCTOS_POR_PAGINA + i;
        const BotonProducto& b = botones[i];

        if (pos < filtrados.size()) {
            const Producto& p = productos[filtrados[pos]];
            b.nombre->setText(p.nombre);
            b.precio->setText(QString("$%1").arg(p.precio, 0, 'f', 2));
            b.codigo->setText(tr("Cód. %1").arg(p.codigo));
            b.boton->setVisible(true);
        }
        else {
            b.boton->setVisible(false);   // hueco vacío en la última página
        }
    }

    ui->lblPagina->setText(tr("Página %1 de %2").arg(paginaActual + 1).arg(totalPaginas));
    ui->btnAnterior->setEnabled(paginaActual > 0);
    ui->btnSiguiente->setEnabled(paginaActual < totalPaginas - 1);
}

// Enter en el buscador por código: el producto con ese código se agrega al carrito.
void MenuPaginacion::agregarPorCodigo()
{
    const QString codigo = ui->leCodigo->text().trimmed();
    if (codigo.isEmpty())
        return;

    const int idx = indiceDeCodigo(codigo);
    if (idx < 0) {
        QMessageBox::warning(this, tr("Producto no encontrado"),
            tr("No existe ningún producto con el código %1.").arg(codigo));
        ui->leCodigo->selectAll();
        return;
    }
    if (productos[idx].stock <= 0) {
        QMessageBox::information(this, tr("Sin existencias"),
            tr("«%1» está agotado.").arg(productos[idx].nombre));
        ui->leCodigo->selectAll();
        return;
    }

    agregarAlCarrito(idx);
    ui->leCodigo->clear();      // listo para escanear/escribir el siguiente código
    ui->leCodigo->setFocus();
}

void MenuPaginacion::paginaAnterior() { --paginaActual; mostrarPagina(); }
void MenuPaginacion::paginaSiguiente() { ++paginaActual; mostrarPagina(); }

// ---------------------------------------------------------------
// Carrito
// ---------------------------------------------------------------

// OJO: los métodos de esta clase se llaman igual que las funciones de datos.h, por eso
// las globales se invocan con "::" (agregarAlCarrito, quitarDelCarrito, vaciarCarrito).

void MenuPaginacion::agregarAlCarrito(int indiceProducto)
{
    const Producto& p = productos[indiceProducto];
    if (p.stock <= 0)
        return;

    ::agregarAlCarrito(p.id, 1);    // devuelve false si ya hay en el carrito todo el stock
    reconstruirCarrito();
}

void MenuPaginacion::cambiarCantidad(const QString& codigo, int delta)
{
    const int idx = indiceDeCodigo(codigo);
    if (idx < 0) {
        reconstruirCarrito();
        return;
    }

    const int id = productos[idx].id;
    if (delta > 0) {
        ::agregarAlCarrito(id, delta);
    }
    else {
        for (ItemCompra& it : carrito) {
            if (it.idProducto != id) continue;

            it.cantidad += delta;
            if (it.cantidad <= 0) {
                quitarDelCarrito(codigo);
                return;
            }
            break;
        }
    }
    reconstruirCarrito();   // también ajusta la cantidad al stock disponible
}

void MenuPaginacion::quitarDelCarrito(const QString& codigo)
{
    const int idx = indiceDeCodigo(codigo);
    if (idx >= 0)
        ::quitarDelCarrito(productos[idx].id);
    reconstruirCarrito();
}

void MenuPaginacion::vaciarCarrito()
{
    ::vaciarCarrito();
    reconstruirCarrito();
}

// Dibuja de nuevo todas las filas del carrito y el total.
void MenuPaginacion::reconstruirCarrito()
{
    // 1) Depurar: fuera lo que ya no existe o no tiene stock; cantidades al máximo disponible
    for (int k = carrito.size() - 1; k >= 0; --k) {
        const int idx = indiceDeId(carrito[k].idProducto);
        if (idx < 0 || productos[idx].stock <= 0) {
            carrito.removeAt(k);
            continue;
        }
        carrito[k].cantidad = qMin(carrito[k].cantidad, productos[idx].stock);
    }

    // 2) Quitar las filas anteriores
    for (QFrame* fila : filasCarrito) {
        ui->layoutItemsCarrito->removeWidget(fila);
        fila->hide();
        fila->deleteLater();   // seguro aunque venga de un clic en un botón de la propia fila
    }
    filasCarrito.clear();

    // 3) Crear una fila por producto del carrito
    double total = 0.0;

    for (const ItemCompra& it : carrito) {
        const Producto& p = productos[indiceDeId(it.idProducto)];
        const double subtotal = it.precio * it.cantidad;   // precio copiado al agregar, igual que al cobrar
        total += subtotal;

        auto* fila = new QFrame(ui->contenedorCarrito);
        fila->setObjectName("itemCarrito");

        auto* vertical = new QVBoxLayout(fila);
        vertical->setContentsMargins(14, 12, 14, 12);
        vertical->setSpacing(8);

        // Arriba: nombre + botón para quitar
        auto* arriba = new QHBoxLayout;
        auto* lblNombre = new QLabel(p.nombre, fila);
        lblNombre->setObjectName("itemNombre");
        lblNombre->setWordWrap(true);

        auto* btnQuitar = new QPushButton(QStringLiteral("\u2715"), fila);
        btnQuitar->setObjectName("btnQuitar");
        btnQuitar->setCursor(Qt::PointingHandCursor);
        btnQuitar->setToolTip(tr("Quitar del carrito"));

        arriba->addWidget(lblNombre, 1);
        arriba->addWidget(btnQuitar, 0, Qt::AlignTop);

        // Abajo: [−] cantidad [+]            subtotal
        auto* abajo = new QHBoxLayout;
        abajo->setSpacing(8);

        auto* btnMenos = new QPushButton(QStringLiteral("\u2212"), fila);
        btnMenos->setObjectName("btnCantidad");
        btnMenos->setCursor(Qt::PointingHandCursor);

        auto* lblCantidad = new QLabel(QString::number(it.cantidad), fila);
        lblCantidad->setObjectName("itemCantidad");
        lblCantidad->setAlignment(Qt::AlignCenter);

        auto* btnMas = new QPushButton("+", fila);
        btnMas->setObjectName("btnCantidad");
        btnMas->setCursor(Qt::PointingHandCursor);
        btnMas->setEnabled(it.cantidad < p.stock);   // sin más unidades disponibles

        auto* lblSubtotal = new QLabel(QString("$%1").arg(subtotal, 0, 'f', 2), fila);
        lblSubtotal->setObjectName("itemSubtotal");

        abajo->addWidget(btnMenos);
        abajo->addWidget(lblCantidad);
        abajo->addWidget(btnMas);
        abajo->addStretch();
        abajo->addWidget(lblSubtotal);

        vertical->addLayout(arriba);
        vertical->addLayout(abajo);

        const QString codigo = p.codigo;
        connect(btnMenos, &QPushButton::clicked, this, [this, codigo] { cambiarCantidad(codigo, -1); });
        connect(btnMas, &QPushButton::clicked, this, [this, codigo] { cambiarCantidad(codigo, +1); });
        connect(btnQuitar, &QPushButton::clicked, this, [this, codigo] { quitarDelCarrito(codigo); });

        // Antes del espaciador final del layout
        ui->layoutItemsCarrito->insertWidget(ui->layoutItemsCarrito->count() - 1, fila);
        filasCarrito.append(fila);
    }

    // 4) Total y estado de los botones
    ui->lblCarritoVacio->setVisible(carrito.isEmpty());
    ui->lblTotal->setText(QString("$%1").arg(total, 0, 'f', 2));
    ui->btnPagar->setEnabled(!carrito.isEmpty());
}

// "Pagar" abre la ventana Carrito, que cobra (efectivo, tarjeta o cuotas), descuenta el
// stock, guarda la compra en el historial y muestra el recibo.
void MenuPaginacion::pagar()
{
    if (carrito.isEmpty())
        return;

    Carrito* ventana = new Carrito();
    ventana->setAttribute(Qt::WA_DeleteOnClose);

    // Tras pagar (o quitar algo en la ventana): refrescar catálogo y panel sin perder la página
    connect(ventana, &Carrito::carritoModificado, this, [this]() {
        const int pagina = paginaActual;
        aplicarFiltros();
        paginaActual = pagina;
        mostrarPagina();
        reconstruirCarrito();
    });
    // Al cerrar la ventana el panel debe reflejar el carrito tal como quedó
    connect(ventana, &QObject::destroyed, this, [this]() { reconstruirCarrito(); });

    ventana->setWindowModality(Qt::ApplicationModal);
    ventana->show();
}
