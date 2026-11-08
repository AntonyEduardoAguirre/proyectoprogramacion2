#include "VentanaAdmin.h"

#include <QLabel>

VentanaAdmin::VentanaAdmin(const QString& usuario, QWidget* parent)
	: QWidget(parent)
{
	setWindowTitle(QStringLiteral("Página de administrador"));
	auto* etiqueta = new QLabel(QStringLiteral("[Placeholder] Administrador: %1").arg(usuario), this);
	etiqueta->setAlignment(Qt::AlignCenter);
	resize(360, 180);
}
