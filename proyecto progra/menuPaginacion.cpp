#include "clases/menuPaginacion.h"
#include "ui_menuPaginacion.h"  // generado por uic a partir de menuPaginacion.ui
#include "clases/datos.h"

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
    // El código solo admite hasta 6 dígitos.
    ui->leCodigo->setValidator(
        new QRegularExpressionValidator(QRegularExpression("\\d{0,6}"), this));

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

void MenuPaginacion::agregarAlCarrito(int indiceProducto)
{
    const Producto& p = productos[indiceProducto];
    if (p.stock <= 0)
        return;

    for (ItemCarrito& it : carrito) {
        if (it.codigo == p.codigo) {
            if (it.cantidad < p.stock)      // no se puede pedir más de lo que hay
                ++it.cantidad;
            reconstruirCarrito();
            return;
        }
    }

    carrito.append(ItemCarrito{ p.codigo, 1 });
    reconstruirCarrito();
}

void MenuPaginacion::cambiarCantidad(const QString& codigo, int delta)
{
    for (ItemCarrito& it : carrito) {
        if (it.codigo != codigo) continue;

        it.cantidad += delta;
        if (it.cantidad <= 0) {
            quitarDelCarrito(codigo);
            return;
        }
        break;
    }
    reconstruirCarrito();   // también ajusta la cantidad al stock disponible
}

void MenuPaginacion::quitarDelCarrito(const QString& codigo)
{
    for (int k = 0; k < carrito.size(); ++k) {
        if (carrito[k].codigo == codigo) {
            carrito.removeAt(k);
            break;
        }
    }
    reconstruirCarrito();
}

void MenuPaginacion::vaciarCarrito()
{
    carrito.clear();
    reconstruirCarrito();
}

// Dibuja de nuevo todas las filas del carrito y el total.
void MenuPaginacion::reconstruirCarrito()
{
    // 1) Depurar: fuera lo que ya no existe o no tiene stock; cantidades al máximo disponible
    for (int k = carrito.size() - 1; k >= 0; --k) {
        const int idx = indiceDeCodigo(carrito[k].codigo);
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

    for (const ItemCarrito& it : carrito) {
        const Producto& p = productos[indiceDeCodigo(it.codigo)];
        const double subtotal = p.precio * it.cantidad;
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

        const QString codigo = it.codigo;
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

void MenuPaginacion::pagar()
{
    if (carrito.isEmpty())
        return;

    double total = 0.0;
    int unidades = 0;
    for (const ItemCarrito& it : carrito) {
        const int idx = indiceDeCodigo(it.codigo);
        if (idx < 0) continue;
        total += productos[idx].precio * it.cantidad;
        unidades += it.cantidad;
    }

    const auto respuesta = QMessageBox::question(
        this, tr("Confirmar compra"),
        tr("Vas a pagar $%1 por %2 artículo(s). ¿Continuar?")
        .arg(total, 0, 'f', 2).arg(unidades),
        QMessageBox::Yes | QMessageBox::No);

    if (respuesta != QMessageBox::Yes)
        return;

    // Descontar del inventario lo que se compró y guardar
    for (const ItemCarrito& it : carrito) {
        const int idx = indiceDeCodigo(it.codigo);
        if (idx >= 0)
            productos[idx].stock = qMax(0, productos[idx].stock - it.cantidad);
    }
    guardarProductos();

    carrito.clear();

    QMessageBox::information(this, tr("Compra realizada"), tr("¡Gracias por tu compra!"));

    // Refrescar el catálogo (los productos agotados desaparecen) sin perder la página
    const int pagina = paginaActual;
    aplicarFiltros();
    paginaActual = pagina;
    mostrarPagina();

    reconstruirCarrito();
}
