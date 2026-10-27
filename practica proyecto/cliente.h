#ifndef CLIENTE_H
#define CLIENTE_H

#include <QString>
#include <QJsonObject>
#include <string>

class Cliente
{
public:
    QString nombre;
    QString contrasenna;

    Cliente() : contrasenna("") {}

    QJsonObject toJson() const
    {
        QJsonObject json;
        json["nombre"] = nombre;
        json["contrasenna"] = contrasenna;
        return json;
    }

    void fromJson(const QJsonObject& json)
    {
        nombre = json["nombre"].toString();
        contrasenna = json["contrasenna"].toString();
    }

    void setNombre(const QString& nombre) { this->nombre = nombre; }
    void setContrasenna(const QString& contrasenna) { this->contrasenna = contrasenna; }

    std::string getNombre() const { return nombre.toStdString(); }
	std::string getContrasenna() const { return contrasenna.toStdString(); }
};

#endif