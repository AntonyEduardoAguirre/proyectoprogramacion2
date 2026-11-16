#include "clases/CalculadoraCambio.h"

#include <cmath>

CalculadoraCambio::Resultado CalculadoraCambio::calcular(double total, double recibido)
{
	const QString totalInvalido = QStringLiteral("El total a pagar debe ser mayor que cero.");
	const QString recibidoInvalido = QStringLiteral("El efectivo recibido debe ser mayor que cero.");

	if (!std::isfinite(total) || total <= 0.0) {
		return {false, 0.0, totalInvalido};
	}

	if (!std::isfinite(recibido) || recibido <= 0.0) {
		return {false, 0.0, recibidoInvalido};
	}

	const qint64 totalCentimos = qRound64(total * 100.0);
	const qint64 recibidoCentimos = qRound64(recibido * 100.0);
	const qint64 diferenciaCentimos = recibidoCentimos - totalCentimos;

	if (totalCentimos <= 0) {
		return {false, 0.0, totalInvalido};
	}

	if (recibidoCentimos <= 0) {
		return {false, 0.0, recibidoInvalido};
	}

	if (diferenciaCentimos < 0) {
		const double faltante = -diferenciaCentimos / 100.0;
		return {
			false,
			0.0,
			QStringLiteral("Pago insuficiente. Faltan ₡ %1").arg(faltante, 0, 'f', 2)
		};
	}

	return {true, diferenciaCentimos / 100.0, QString()};
}
