#ifndef PRODUCTO_H
#define PRODUCTO_H

#include <QString>
#include <QJsonObject>

class Producto
{
public:
    QString categoria;
    int id;
    QString codigo;      // código numérico de al menos 3 dígitos, único por producto
    QString nombre;
    double precio;
    int stock;

    Producto() : id(0), precio(0.0), stock(0) {}

    // Un código válido contiene al menos 3 dígitos y puede comenzar con cero.
    static bool codigoValido(const QString& c)
    {
        if (c.size() < 3)
            return false;
        for (const QChar ch : c)
            if (ch.unicode() < '0' || ch.unicode() > '9')
                return false;
        return true;
    }

    QJsonObject toJson() const
    {
        QJsonObject json;
        json["categoria"] = categoria;
        json["id"] = id;
        json["codigo"] = codigo;
        json["nombre"] = nombre;
        json["precio"] = precio;
        json["stock"] = stock;

        return json;
    }

    void fromJson(const QJsonObject& json)
    {
        categoria = json["categoria"].toString();
        id = json["id"].toInt();
        codigo = json["codigo"].toString();   // vacío si el JSON es anterior al código
        nombre = json["nombre"].toString();
        precio = json["precio"].toDouble();
        stock = json["stock"].toInt();
    }
};

#endif
