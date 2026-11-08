#include "VentanaCarrito.h"

#include <QLabel>

VentanaCarrito::VentanaCarrito(const QString& usuario, QWidget* parent)
	: QWidget(parent)
{
	setWindowTitle(QStringLiteral("Carrito de compras"));
	auto* etiqueta = new QLabel(QStringLiteral("[Placeholder] Carrito: %1").arg(usuario), this);
	etiqueta->setAlignment(Qt::AlignCenter);
	resize(360, 180);
}
