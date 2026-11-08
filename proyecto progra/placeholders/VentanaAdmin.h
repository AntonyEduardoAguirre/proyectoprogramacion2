#pragma once

#include <QWidget>

class VentanaAdmin : public QWidget
{
	Q_OBJECT

public:
	explicit VentanaAdmin(const QString& usuario, QWidget* parent = nullptr);
};
