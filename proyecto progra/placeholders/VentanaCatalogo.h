#pragma once

#include <QWidget>

class VentanaCatalogo : public QWidget
{
	Q_OBJECT

public:
	explicit VentanaCatalogo(const QString& usuario, QWidget* parent = nullptr);
};
