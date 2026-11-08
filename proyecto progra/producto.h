#ifndef PRODUCTO_H
#define PRODUCTO_H

#include <QString>
#include <QJsonObject>

class Producto
{
public:
    QString categoria;
    int id;
    QString nombre;
    double precio;
    int stock;

    Producto() : precio(0.0), stock(0), id(0) {}

    QJsonObject toJson() const
    {
        QJsonObject json;
        json["categoria"] = categoria;
        json["id"] = id;
        json["nombre"] = nombre;
        json["precio"] = precio;
		json["stock"] = stock;
		
        return json;
    }

    void fromJson(const QJsonObject& json)
    {
        categoria = json["categoria"].toString();
        id = json["id"].toInt();
        nombre = json["nombre"].toString();
        precio = json["precio"].toDouble();
		stock = json["stock"].toInt();
		
    }
};

#endif