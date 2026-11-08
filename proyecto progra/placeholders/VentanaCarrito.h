#pragma once

#include <QWidget>

class VentanaCarrito : public QWidget
{
	Q_OBJECT

public:
	explicit VentanaCarrito(const QString& usuario, QWidget* parent = nullptr);
};
