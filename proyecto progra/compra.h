#ifndef COMPRA_H
#define COMPRA_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

// Una línea del carrito o de una compra: producto + cantidad
// (el nombre y el precio se copian al comprar, para que el
//  historial quede fiel aunque el catálogo cambie después)
class ItemCompra
{
public:
    int idProducto = 0;
    QString nombre;
    double precio = 0.0;
    int cantidad = 0;

    QJsonObject toJson() const
    {
        QJsonObject json;
        json["idProducto"] = idProducto;
        json["nombre"] = nombre;
        json["precio"] = precio;
        json["cantidad"] = cantidad;
        return json;
    }

    void fromJson(const QJsonObject& json)
    {
        idProducto = json["idProducto"].toInt(0);
        nombre = json["nombre"].toString();
        precio = json["precio"].toDouble(0.0);
        cantidad = json["cantidad"].toInt(0);
    }
};

// Una compra completa: cliente + fecha + líneas + total
class Compra
{
public:
    int clienteId = 0;
    QString fecha;        // texto "dd/MM/yyyy hh:mm"
    double total = 0.0;
    QList<ItemCompra> items;

    QJsonObject toJson() const
    {
        QJsonObject json;
        json["clienteId"] = clienteId;
        json["fecha"] = fecha;
        json["total"] = total;

        QJsonArray arregloItems;
        for (const ItemCompra& item : items)
            arregloItems.append(item.toJson());
        json["items"] = arregloItems;

        return json;
    }

    void fromJson(const QJsonObject& json)
    {
        clienteId = json["clienteId"].toInt(0);
        fecha = json["fecha"].toString();
        total = json["total"].toDouble(0.0);

        items.clear();
        for (const QJsonValue& valor : json["items"].toArray()) {
            ItemCompra item;
            item.fromJson(valor.toObject());
            items.append(item);
        }
    }
};

#endif
