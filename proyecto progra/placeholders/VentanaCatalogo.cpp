#include "VentanaCatalogo.h"

#include <QLabel>

VentanaCatalogo::VentanaCatalogo(const QString& usuario, QWidget* parent)
	: QWidget(parent)
{
	setWindowTitle(QStringLiteral("Catálogo"));
	auto* etiqueta = new QLabel(QStringLiteral("[Placeholder] Catálogo: %1").arg(usuario), this);
	etiqueta->setAlignment(Qt::AlignCenter);
	resize(360, 180);
}
