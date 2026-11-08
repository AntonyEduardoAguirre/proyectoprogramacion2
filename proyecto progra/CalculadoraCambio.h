#pragma once

#include <QString>
#include <QtGlobal>

// Contiene únicamente la lógica para calcular el cambio.
class CalculadoraCambio
{
public:
	struct Resultado
	{
		bool valido;
		double cambio;
		QString error;
	};

	static Resultado calcular(double total, double recibido);
};
