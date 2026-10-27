#ifndef PRODUCTO_H
#define PRODUCTO_H

#include <QString>
#include <QJsonObject>

class Producto
{
public:
    QString nombre;
    QString categoria;
    double precio;
    int id;
    int stock;

    Producto() : precio(0.0), stock(0), id(0) {}

    QJsonObject toJson() const
    {
        QJsonObject json;
        json["nombre"] = nombre;
        json["categoria"] = categoria;
        json["precio"] = precio;
		json["stock"] = stock;
		json["id"] = id;
        return json;
    }

    void fromJson(const QJsonObject& json)
    {
        nombre = json["nombre"].toString();
		categoria = json["categoria"].toString();   
        precio = json["precio"].toDouble();
		stock = json["stock"].toInt();
		id = json["id"].toInt();
    }
};

#endif