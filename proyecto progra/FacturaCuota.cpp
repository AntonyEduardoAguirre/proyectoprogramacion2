#include "clases/FacturaCuota.h"

#include <QJsonArray>

QJsonObject FacturaCuota::toJson() const
{
    QJsonObject json;
    json["numero_factura"] = numeroTexto();
    json["fecha_finalizacion"] = fechaFinalizacion;
    json["cliente"] = cliente;
    json["caja"] = caja;

    QJsonArray productosJson;
    for (const ItemCompra& item : productos)
        productosJson.append(item.toJson());
    json["producto"] = productosJson;

    json["fecha_compra"] = fechaCompra;
    json["total_con_aumento"] = totalConAumento;
    json["total_pagado"] = totalPagado;

    QJsonArray pagosJson;
    for (const PagoCuota& pago : pagos) {
        QJsonObject pagoJson;
        pagoJson["fecha"] = pago.fecha;
        pagoJson["monto"] = pago.monto;
        pagoJson["enviado_admin"] = pago.enviadoAdmin;
        pagosJson.append(pagoJson);
    }
    json["pagos"] = pagosJson;
    json["enviada_admin"] = enviadaAdmin;
    json["fecha_envio_admin"] = fechaEnvioAdmin;

    return json;
}

void FacturaCuota::fromJson(const QJsonObject& json)
{
    const QString textoNumero = json["numero_factura"].toString();
    numero = textoNumero.startsWith(QStringLiteral("F"))
        ? textoNumero.mid(2).toInt()   // "F-000001" -> 1
        : textoNumero.toInt();         // compatibilidad con valores numéricos
    fechaFinalizacion = json["fecha_finalizacion"].toString();
    cliente = json["cliente"].toString();
    caja = json["caja"].toString();
    fechaCompra = json["fecha_compra"].toString();
    totalConAumento = json["total_con_aumento"].toDouble(0.0);
    totalPagado = json["total_pagado"].toDouble(0.0);
    enviadaAdmin = json["enviada_admin"].toBool(false);
    fechaEnvioAdmin = json["fecha_envio_admin"].toString();

    productos.clear();
    for (const QJsonValue& valor : json["producto"].toArray()) {
        ItemCompra item;
        item.fromJson(valor.toObject());
        productos.append(item);
    }

    pagos.clear();
    for (const QJsonValue& valor : json["pagos"].toArray()) {
        const QJsonObject pagoJson = valor.toObject();
        PagoCuota pago;
        pago.fecha = pagoJson["fecha"].toString();
        pago.monto = pagoJson["monto"].toDouble(0.0);
        pago.enviadoAdmin = pagoJson["enviado_admin"].toBool(false);
        pagos.append(pago);
    }
}
