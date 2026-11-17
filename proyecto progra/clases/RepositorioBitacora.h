#pragma once

#include "CierreDia.h"

#include <QList>
#include <QString>

// Lectura y escritura de datos/bitacora.json. El menú admin todavía no
// existe; esta clase queda como contrato para que lo use después con
// todos() y existeCierre().
class RepositorioBitacora
{
public:
    bool guardar(const CierreDia& cierre, QString* error = nullptr) const;
    bool todos(QList<CierreDia>* cierres, QString* error = nullptr) const;
    bool existeCierre(const QDate& fecha, QString* error = nullptr) const;

private:
    static QString rutaArchivoBitacora();
};
