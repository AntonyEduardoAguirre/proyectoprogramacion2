#include "clases/CuotaCompletada.h"

#include <QLabel>
#include <QPushButton>

CuotaCompletada::CuotaCompletada(const Compra& compra, const QString& cliente, const QString& fecha,
                                 double totalSinAumento, double totalConAumento,
                                 double pagoMensual, int mesesRestantes, QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setWindowState(Qt::WindowMaximized);

    // ticket con los productos comprados, como en el recibo
    for (const ItemCompra& item : compra.items) {
        auto* fila = new QLabel(
            QStringLiteral("%1 × %2\n$%3 c/u  —  $%4")
                .arg(item.cantidad)
                .arg(item.nombre)
                .arg(item.precio, 0, 'f', 2)
                .arg(item.precio * item.cantidad, 0, 'f', 2),
            this);
        fila->setObjectName(QStringLiteral("lblItem"));
        ui.layoutItems->addWidget(fila);
    }

    // información completa de la cuota, como un recibo
    auto* lblCliente = new QLabel(
        QStringLiteral("Cliente: %1").arg(cliente), this);
    lblCliente->setObjectName(QStringLiteral("lblPago"));
    lblCliente->setWordWrap(true);

    auto* lblFecha = new QLabel(
        QStringLiteral("Fecha: %1").arg(fecha), this);
    lblFecha->setObjectName(QStringLiteral("lblPago"));
    lblFecha->setWordWrap(true);

    auto* lblTotalSin = new QLabel(
        QStringLiteral("Precio total sin aumento: $%1")
            .arg(QString::number(totalSinAumento, 'f', 2)), this);
    lblTotalSin->setObjectName(QStringLiteral("lblPago"));

    auto* lblAumento = new QLabel(
        QStringLiteral("Aumento: 0.5% por cada mes → %1% de recargo")
            .arg(QString::number(0.5 * compra.meses, 'f', 1)), this);
    lblAumento->setObjectName(QStringLiteral("lblPago"));

    auto* lblTotal = new QLabel(
        QStringLiteral("Precio total con el aumento: $%1")
            .arg(QString::number(totalConAumento, 'f', 2)), this);
    lblTotal->setObjectName(QStringLiteral("lblTotal"));
    lblTotal->setWordWrap(true);

    auto* lblPlazo = new QLabel(
        QStringLiteral("Plazo elegido: %1 meses  —  quedan %2 pagos mensuales")
            .arg(compra.meses)
            .arg(mesesRestantes), this);
    lblPlazo->setObjectName(QStringLiteral("lblPago"));
    lblPlazo->setWordWrap(true);

    auto* lblPagoMensual = new QLabel(
        QStringLiteral("Cantidad mensual a pagar por los %1 meses restantes: $%2")
            .arg(mesesRestantes)
            .arg(QString::number(pagoMensual, 'f', 2)), this);
    lblPagoMensual->setObjectName(QStringLiteral("lblPago"));
    lblPagoMensual->setWordWrap(true);

    // el nombre del cliente otra vez, al pie del recibo
    auto* lblClientePie = new QLabel(
        QStringLiteral("Cliente: %1").arg(cliente), this);
    lblClientePie->setObjectName(QStringLiteral("lblPago"));
    lblClientePie->setWordWrap(true);

    ui.layoutItems->addWidget(lblCliente);
    ui.layoutItems->addWidget(lblFecha);
    ui.layoutItems->addWidget(lblTotalSin);
    ui.layoutItems->addWidget(lblAumento);
    ui.layoutItems->addWidget(lblTotal);
    ui.layoutItems->addWidget(lblPlazo);
    ui.layoutItems->addWidget(lblPagoMensual);
    ui.layoutItems->addWidget(lblClientePie);
    ui.layoutItems->addStretch();

    connect(ui.btnListo, &QPushButton::clicked, this, [this]() {
        emit cuotaCerrada();
        close();
        });
}

CuotaCompletada::~CuotaCompletada()
{
}
