#pragma once

#include <QWidget>

class VentanaCuenta : public QWidget
{
	Q_OBJECT

public:
	explicit VentanaCuenta(const QString& usuario, QWidget* parent = nullptr);
};
