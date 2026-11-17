#include "clases/VentanaDetalleCuota.h"

#include <QDateTime>
#include <QKeyEvent>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>

#include <algorithm>

namespace {

// Da el estilo del menú a los carteles que salen al completar un pago.
void aplicarEstiloMensaje(QMessageBox* caja)
{
    caja->setStyleSheet(QStringLiteral(
        "QMessageBox { background-color: #F5F3EE; }"
        "QMessageBox QLabel { color: #292A28; font-family: 'Segoe UI'; font-size: 13px; }"
        "QMessageBox QPushButton { background-color: #899B83; color: #FFFFFF; border: none; "
        "border-radius: 8px; min-height: 34px; padding: 0 16px; font-family: 'Segoe UI'; "
        "font-weight: 600; }"
        "QMessageBox QPushButton:hover { background-color: #788B72; }"
        "QMessageBox QPushButton:pressed { background-color: #6D8067; }"));
}

} // namespace

VentanaDetalleCuota::VentanaDetalleCuota(const Cuota& cuota, int indice,
                                         const QList<ItemCompra>& productos,
                                         QWidget* parent, bool soloLectura)
    : QWidget(parent), cuota_(cuota), indice_(indice), productos_(productos),
      soloLectura_(soloLectura)
{
    ui.setupUi(this);

    connect(ui.btnCompletarPago, &QPushButton::clicked,
            this, &VentanaDetalleCuota::completarPago);
    connect(ui.btnRegresar, &QPushButton::clicked, this, &QWidget::close);

    if (soloLectura_) {
        // Factura completada: se oculta toda la sección de pago.
        ui.lblSeccionPago->setVisible(false);
        ui.lblPagoEsteMes->setVisible(false);
        ui.lblEstadoPago->setVisible(false);
        ui.btnCompletarPago->setVisible(false);
    }

    mostrarCuota();
}

VentanaDetalleCuota::~VentanaDetalleCuota()
{
}

void VentanaDetalleCuota::keyPressEvent(QKeyEvent* evento)
{
    if (evento->key() == Qt::Key_Escape) {
        close();
        return;
    }
    QWidget::keyPressEvent(evento);
}

void VentanaDetalleCuota::mostrarCuota()
{
    ui.lblCliente->setText(QStringLiteral("Cliente: %1").arg(cuota_.cliente));
    ui.lblCaja->setText(QStringLiteral("Caja: %1")
                            .arg(cuota_.caja.isEmpty() ? QStringLiteral("—") : cuota_.caja));
    ui.lblFecha->setText(QStringLiteral("Fecha en que se realizó: %1").arg(cuota_.fecha));
    ui.lblTotalSinAumento->setText(
        QStringLiteral("Precio total sin aumento: $%1")
            .arg(QString::number(cuota_.totalSinAumento, 'f', 2)));
    ui.lblAumento->setText(
        QStringLiteral("Aumento: %1% de recargo")
            .arg(QString::number(cuota_.porcentajeAumento, 'f', 1)));
    ui.lblTotalConAumento->setText(
        QStringLiteral("Precio total con aumento: $%1")
            .arg(QString::number(cuota_.totalConAumento, 'f', 2)));
    ui.lblPlazo->setText(QStringLiteral("Plazo elegido: %1 meses").arg(cuota_.meses));
    ui.lblCuotasPagadas->setText(
        QStringLiteral("Cuotas pagadas: %1").arg(cuota_.cuotasPagadas()));
    ui.lblCuotasPendientes->setText(
        QStringLiteral("Cuotas pendientes: %1").arg(cuota_.cuotasPendientes()));
    ui.lblSaldo->setText(
        QStringLiteral("Saldo pendiente: $%1")
            .arg(QString::number(cuota_.saldoPendiente(), 'f', 2)));
    ui.lblFaltaPagar->setText(
        QStringLiteral("Falta por pagar: $%1")
            .arg(QString::number(cuota_.saldoPendiente(), 'f', 2)));
    ui.lblPagado->setText(
        QStringLiteral("Pagado: $%1")
            .arg(QString::number(cuota_.totalPagado(), 'f', 2)));
    ui.lblPagoMensual->setText(
        QStringLiteral("Cuota mensual: $%1")
            .arg(QString::number(cuota_.pagoMensual, 'f', 2)));

    ui.listaProductos->clear();
    if (productos_.isEmpty()) {
        ui.listaProductos->addItem(QStringLiteral("Detalle de productos no disponible."));
    } else {
        for (const ItemCompra& item : productos_) {
            ui.listaProductos->addItem(
                QStringLiteral("%1 × %2 — $%3")
                    .arg(QString::number(item.cantidad), item.nombre,
                         QString::number(item.precio * item.cantidad, 'f', 2)));
        }
    }

    if (soloLectura_) {
        if (cuota_.factura.numero > 0) {
            ui.lblFactura->setVisible(true);
            ui.lblFactura->setText(
                QStringLiteral("Factura %1 — Finalizada: %2 · Total pagado: $%3")
                    .arg(cuota_.factura.numeroTexto(), cuota_.factura.fechaFinalizacion,
                         QString::number(cuota_.factura.totalPagado, 'f', 2)));
        }
        return;
    }

    const double pago = pagoDeEsteMes();
    if (pago > 0.0) {
        ui.lblPagoEsteMes->setText(
            QStringLiteral("Pago de este mes: $%1")
                .arg(QString::number(pago, 'f', 2)));
    } else {
        ui.lblPagoEsteMes->setText(QStringLiteral("Pago de este mes: cuota completada"));
    }

    if (!cuota_.activa()) {
        ui.btnCompletarPago->setEnabled(false);
        ui.btnCompletarPago->setText(QStringLiteral("Cuota completada"));
        mostrarEstado(QStringLiteral("Esta cuota ya está completada."), false);
    } else if (pago <= 0.0) {
        ui.btnCompletarPago->setEnabled(false);
        ui.btnCompletarPago->setText(QStringLiteral("Completar pago de este mes"));
        mostrarEstado(QStringLiteral("La cuota mensual no es válida."), true);
    } else {
        ui.btnCompletarPago->setEnabled(true);
        ui.btnCompletarPago->setText(QStringLiteral("Completar pago de este mes"));
        mostrarEstado(QString(), false);
    }
}

void VentanaDetalleCuota::mostrarEstado(const QString& texto, bool error)
{
    ui.lblEstadoPago->setProperty("error", error);
    ui.lblEstadoPago->style()->unpolish(ui.lblEstadoPago);
    ui.lblEstadoPago->style()->polish(ui.lblEstadoPago);
    ui.lblEstadoPago->setText(texto);
}

void VentanaDetalleCuota::completarPago()
{
    const double monto = pagoDeEsteMes();
    if (monto <= 0.0)
        return;

    const double resto = cuota_.saldoPendiente() - monto;
    QMessageBox confirmacion(this);
    confirmacion.setWindowTitle(QStringLiteral("Completar pago"));
    confirmacion.setText(
        QStringLiteral("¿Registrar este pago?\n\nCliente: %1\nMonto: $%2\n\n"
                       "Falta por pagar después de este pago: $%3")
            .arg(cuota_.cliente, QString::number(monto, 'f', 2),
                 QString::number(resto > 0.0 ? resto : 0.0, 'f', 2)));
    confirmacion.setIcon(QMessageBox::Question);
    confirmacion.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmacion.setDefaultButton(QMessageBox::No);
    aplicarEstiloMensaje(&confirmacion);
    if (confirmacion.exec() != QMessageBox::Yes)
        return;

    RepositorioCuotas repositorio;
    QString error;
    const QString fecha = QDateTime::currentDateTime().toString(
        QStringLiteral("dd/MM/yyyy hh:mm:ss"));

    if (!repositorio.registrarPago(indice_, fecha, monto, &error)) {
        mostrarEstado(error, true);
        QMessageBox aviso(this);
        aviso.setWindowTitle(QStringLiteral("Completar pago"));
        aviso.setText(error);
        aviso.setIcon(QMessageBox::Warning);
        aplicarEstiloMensaje(&aviso);
        aviso.exec();
        return;
    }

    QList<Cuota> cuotas;
    if (!repositorio.cargar(&cuotas, &error)
        || indice_ < 0 || indice_ >= cuotas.size()) {
        mostrarEstado(error.isEmpty()
                          ? QStringLiteral("El pago se guardó, pero no se pudo refrescar la cuota.")
                          : QStringLiteral("El pago se guardó, pero no se pudo refrescar: %1")
                                .arg(error),
                      true);
        return;
    }

    cuota_ = cuotas.at(indice_);

    // Al llegar el saldo a cero se genera la factura y se guarda antes de enviar.
    if (cuota_.saldoPendiente() <= 0.004 && cuota_.factura.numero == 0) {
        FacturaCuota factura;
        factura.numero = repositorio.siguienteNumeroFactura(&error);
        if (factura.numero <= 0) {
            QMessageBox aviso(this);
            aviso.setWindowTitle(QStringLiteral("Completar pago"));
            aviso.setText(error.isEmpty()
                              ? QStringLiteral("No se pudo asignar el número de factura.")
                              : error);
            aviso.setIcon(QMessageBox::Warning);
            aplicarEstiloMensaje(&aviso);
            aviso.exec();
            return;
        }
        factura.fechaFinalizacion = fecha;
        factura.cliente = cuota_.cliente;
        factura.caja = cuota_.caja;
        factura.productos = productos_;
        factura.fechaCompra = cuota_.fecha;
        factura.totalConAumento = cuota_.totalConAumento;
        factura.totalPagado = cuota_.totalPagado();
        factura.pagos = cuota_.pagos;
        factura.enviadaAdmin = false;   // pendiente de confirmación del admin

        if (!repositorio.completar(indice_, factura, &error)) {
            mostrarEstado(error, true);
            QMessageBox aviso(this);
            aviso.setWindowTitle(QStringLiteral("Completar pago"));
            aviso.setText(error);
            aviso.setIcon(QMessageBox::Warning);
            aplicarEstiloMensaje(&aviso);
            aviso.exec();
            return;
        }

        cuota_.factura = factura;
        cuota_.completada = true;
        cuota_.mesesRestantes = 0;

        QMessageBox exito(this);
        exito.setWindowTitle(QStringLiteral("Cuota completada"));
        exito.setText(
            QStringLiteral("La cuota ha sido completada con éxito\n\nCliente: %1\n"
                           "Falta por pagar: $0.00")
                .arg(cuota_.cliente));
        exito.setIcon(QMessageBox::Information);
        aplicarEstiloMensaje(&exito);
        exito.exec();

        // La cuota NO se envía al admin aquí: se enviará en el Cierre.
        close();   // vuelve a la lista de VentanaCuotas
        return;
    }

    mostrarCuota();

    if (cuota_.saldoPendiente() > 0.004) {
        QMessageBox aviso(this);
        aviso.setWindowTitle(QStringLiteral("Pago registrado"));
        aviso.setText(
            QStringLiteral("El pago se registró correctamente.\n\n"
                           "Falta por pagar: $%1")
                .arg(QString::number(cuota_.saldoPendiente(), 'f', 2)));
        aviso.setIcon(QMessageBox::Information);
        aplicarEstiloMensaje(&aviso);
        aviso.exec();
    }
}

double VentanaDetalleCuota::pagoDeEsteMes() const
{
    if (!cuota_.activa())
        return 0.0;
    return std::min(cuota_.pagoMensual, cuota_.saldoPendiente());
}
