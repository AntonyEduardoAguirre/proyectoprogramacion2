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
	int id; // Agregamos un atributo id para identificar al cliente
	float saldo; // Agregamos un atributo saldo para almacenar el saldo del cliente

    Cliente() : contrasenna(""), saldo(0.0f) {}

    QJsonObject toJson() const
    {
        QJsonObject json;
        json["id"] = id;
        json["nombre"] = nombre;
        json["contrasenna"] = contrasenna;
        json["saldo"] = saldo;
        return json;
    }

    void fromJson(const QJsonObject& json)
    {
        id = json["id"].toInt(0);        // <- debe estar
        nombre = json["nombre"].toString();
        contrasenna = json["contrasenna"].toString();
        saldo = json["saldo"].toDouble(0.0);
    }

    void setNombre(const QString& nombre) { this->nombre = nombre; }
    void setContrasenna(const QString& contrasenna) { this->contrasenna = contrasenna; }
    void setId(int id) { this->id = id; }
    void setSaldo(float saldo) { this->saldo = saldo; }

    std::string getNombre() const { return nombre.toStdString(); }
	std::string getContrasenna() const { return contrasenna.toStdString(); }
	int getId() const { return id; }
	float getSaldo() const { return saldo; }
};

#endif