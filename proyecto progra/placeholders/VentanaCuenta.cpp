#include "../clases/VentanaCuenta.h"

#include <QLabel>

VentanaCuenta::VentanaCuenta(const QString& usuario, QWidget* parent)
	: QWidget(parent)
{
	setWindowTitle(QStringLiteral("Cuenta"));
	auto* etiqueta = new QLabel(QStringLiteral("[Placeholder] Cuenta: %1").arg(usuario), this);
	etiqueta->setAlignment(Qt::AlignCenter);
	resize(360, 180);
}
