#include "clases/VentanaCuotas.h"

#include "clases/VentanaDetalleCuota.h"
#include "clases/datos.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QStyle>
#include <QTabWidget>

namespace {

// El JSON de cuotas no guarda los productos, pero la compra original sí
// está en historial.json. Se busca por método, fecha y total.
QList<ItemCompra> buscarProductos(const Cuota& cuota)
{
    for (const Compra& compra : historialCompras) {
        if (compra.metodo != QStringLiteral("Tarjeta (cuotas)"))
            continue;
        if (compra.fecha.left(10) != cuota.fecha.left(10))
            continue;
        if (qAbs(compra.total - cuota.totalConAumento) > 0.01)
            continue;
        return compra.items;
    }
    return {};
}

} // namespace

VentanaCuotas::VentanaCuotas(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    cargarHistorial();

    connect(ui.txtBuscar, &QLineEdit::textChanged, this, &VentanaCuotas::filtrar);
    connect(ui.tabs, &QTabWidget::currentChanged, this, &VentanaCuotas::filtrar);
    connect(ui.btnRegresar, &QPushButton::clicked, this, &QWidget::close);

    auto abrirItem = [this](QListWidgetItem* item) {
        const bool completada = item->listWidget() == ui.listaCompletadas;
        abrirDetalle(item->data(Qt::UserRole).toInt(), completada);
    };
    connect(ui.listaCuotas, &QListWidget::itemClicked, this, abrirItem);
    connect(ui.listaCuotas, &QListWidget::itemActivated, this, abrirItem);
    connect(ui.listaCompletadas, &QListWidget::itemClicked, this, abrirItem);
    connect(ui.listaCompletadas, &QListWidget::itemActivated, this, abrirItem);

    cargarCuotas();
}

VentanaCuotas::~VentanaCuotas()
{
}

void VentanaCuotas::keyPressEvent(QKeyEvent* evento)
{
    if (evento->key() == Qt::Key_Escape) {
        close();
        return;
    }
    QWidget::keyPressEvent(evento);
}

void VentanaCuotas::cargarCuotas()
{
    RepositorioCuotas repositorio;
    QString error;
    if (!repositorio.cargar(&cuotas_, &error)) {
        cuotas_.clear();
        errorCarga_ = error;
        ui.listaCuotas->clear();
        mostrarEstado(errorCarga_, true);
        return;
    }

    errorCarga_.clear();
    filtrar();
}

void VentanaCuotas::filtrar()
{
    ui.listaCuotas->clear();
    ui.listaCompletadas->clear();
    if (!errorCarga_.isEmpty()) {
        mostrarEstado(errorCarga_, true);
        return;
    }

    const QString filtro = ui.txtBuscar->text().trimmed();
    int activas = 0;
    int completadas = 0;

    for (int i = 0; i < cuotas_.size(); ++i) {
        const Cuota& cuota = cuotas_.at(i);
        const bool coincide = filtro.isEmpty()
            || cuota.cliente.contains(filtro, Qt::CaseInsensitive);

        if (cuota.activa()) {
            ++activas;
            if (!coincide)
                continue;

            auto* item = new QListWidgetItem(
                QStringLiteral("%1\nCaja: %2 · Fecha: %3 · Falta $%4")
                    .arg(cuota.cliente,
                         cuota.caja.isEmpty() ? QStringLiteral("—") : cuota.caja,
                         cuota.fecha.left(10),
                         QString::number(cuota.saldoPendiente(), 'f', 2)),
                ui.listaCuotas);
            item->setData(Qt::UserRole, i);
            item->setToolTip(
                QStringLiteral("Saldo pendiente: $%1\nCuota mensual: $%2")
                    .arg(QString::number(cuota.saldoPendiente(), 'f', 2),
                         QString::number(cuota.pagoMensual, 'f', 2)));
        } else if (cuota.factura.numero > 0) {
            ++completadas;
            if (!coincide)
                continue;

            // Las completadas permanecen siempre, también después del Cierre.
            const QString estado = cuota.factura.enviadaAdmin
                ? QStringLiteral("Enviada al admin")
                : QStringLiteral("Pendiente de envío al admin");
            auto* item = new QListWidgetItem(
                QStringLiteral("%1 — Factura %2\n%3 · %4")
                    .arg(cuota.cliente, cuota.factura.numeroTexto(),
                         cuota.factura.fechaFinalizacion.left(10), estado),
                ui.listaCompletadas);
            item->setData(Qt::UserRole, i);
        }
    }

    if (ui.tabs->currentIndex() == 0) {
        if (activas == 0)
            mostrarEstado(QStringLiteral("No hay cuotas activas."), false);
        else if (ui.listaCuotas->count() == 0)
            mostrarEstado(QStringLiteral("Sin resultados para la búsqueda."), false);
        else
            mostrarEstado(QString(), false);
    } else {
        if (completadas == 0)
            mostrarEstado(QStringLiteral("No hay cuotas completadas."), false);
        else if (ui.listaCompletadas->count() == 0)
            mostrarEstado(QStringLiteral("Sin resultados para la búsqueda."), false);
        else
            mostrarEstado(QString(), false);
    }
}

void VentanaCuotas::abrirDetalle(int indice, bool soloLectura)
{
    if (detalleAbierto_ || indice < 0 || indice >= cuotas_.size())
        return;

    detalleAbierto_ = true;
    const Cuota& cuota = cuotas_.at(indice);
    auto* detalle = new VentanaDetalleCuota(cuota, indice, buscarProductos(cuota),
                                             nullptr, soloLectura);
    detalle->setAttribute(Qt::WA_DeleteOnClose);

    connect(detalle, &QObject::destroyed, this, [this]() {
        detalleAbierto_ = false;
        cargarCuotas();
        show();
    });

    hide();
    detalle->show();
}

void VentanaCuotas::mostrarEstado(const QString& texto, bool error)
{
    ui.lblEstado->setProperty("error", error);
    ui.lblEstado->style()->unpolish(ui.lblEstado);
    ui.lblEstado->style()->polish(ui.lblEstado);
    ui.lblEstado->setText(texto);
}
