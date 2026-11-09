#pragma once

#include <QWidget>

class QLabel;
class QDoubleSpinBox;

class VentanaCobro : public QWidget
{
	Q_OBJECT

public:
	explicit VentanaCobro(const QString& usuario, double total = 0.0, QWidget* parent = nullptr);

	void setTotal(double total);

private slots:
	void calcularVuelto();
	void limpiar();

private:
	void mostrarError(const QString& mensaje);

	QString usuario_;
	QDoubleSpinBox* total_;
	QDoubleSpinBox* efectivo_;
	QLabel* resultado_;
};
