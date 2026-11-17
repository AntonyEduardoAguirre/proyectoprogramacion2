#pragma once

#include "FacturaCuota.h"

#include <QDate>
#include <QList>
#include <QString>

// Una compra a cuotas tal como se guarda en datos/cuotas.json.
// Se conservan los campos originales y se agregan "pagos", "completada",
// "caja" y "factura" sin romper los registros anteriores.
struct Cuota
{
    QString cliente;
    QString fecha;          // fecha en que se realizó la cuota
    QString caja;           // usuario que la realizó (sesión iniciada)
    int meses = 0;
    int mesesRestantes = 0;
    double pagoMensual = 0.0;
    double porcentajeAumento = 0.0;
    double totalConAumento = 0.0;
    double totalSinAumento = 0.0;
    bool completada = false;
    QList<PagoCuota> pagos;
    FacturaCuota factura;   // solo se llena al completar la cuota

    double totalPagado() const;
    double saldoPendiente() const;
    int cuotasPagadas() const;
    int cuotasPendientes() const;
    bool activa() const;
    bool pagoHechoEn(const QDate& dia) const;
};

// Marcas que el Cierre aplica a cuotas.json tras verificar la bitácora.
struct MarcaEnvioCuotas
{
    int indiceCuota = -1;
    QList<int> pagos;   // índices (0-based) de los pagos enviados
    bool factura = false;
};

// Única clase que lee y escribe datos/cuotas.json.
class RepositorioCuotas
{
public:
    bool cargar(QList<Cuota>* cuotas, QString* error = nullptr) const;
    bool guardar(const QList<Cuota>& cuotas, QString* error = nullptr) const;
    bool agregar(Cuota cuota, QString* error = nullptr) const;
    bool registrarPago(int indice, const QString& fecha, double monto,
                       QString* error = nullptr) const;
    bool completar(int indice, const FacturaCuota& factura,
                   QString* error = nullptr) const;
    int siguienteNumeroFactura(QString* error = nullptr) const;

    // Identificador estable de un pago (para no enviarlo dos veces).
    static QString idPago(const QString& cliente, const QString& fecha, double monto);
    // Marca pagos y facturas como enviados en una sola escritura con QSaveFile.
    bool marcarEnviados(const QList<MarcaEnvioCuotas>& marcas,
                        const QString& fechaEnvio, QString* error = nullptr) const;

private:
    static QString rutaArchivoCuotas();
};
