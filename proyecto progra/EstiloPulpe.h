#pragma once

#include <QString>

// Estilo compartido basado en la apariencia definida en inicio.ui.
namespace EstiloPulpe
{
inline QString hojaEstilo()
{
	return QStringLiteral(R"(
QWidget#menuPrincipal, QWidget#ventanaCobro {
	background-color: #F5F3EE;
}

QFrame#tarjeta {
	background-color: #FFFFFF;
	border: 1px solid #EAE8E2;
	border-radius: 18px;
}

QLabel#tituloPrincipal, QLabel#tituloCobro {
	color: #292A28;
	font-family: Georgia;
	font-size: 29px;
	font-weight: 400;
}

QLabel#subtitulo, QLabel#etiquetaUsuario {
	color: #92918B;
	font-family: "Segoe UI";
	font-size: 12px;
}

QLabel#etiquetaFormulario {
	color: #292A28;
	font-family: "Segoe UI";
	font-size: 12px;
	font-weight: 600;
}

QPushButton {
	min-height: 46px;
	border-radius: 8px;
	padding: 0 13px;
	font-family: "Segoe UI";
	font-size: 13px;
	font-weight: 600;
}

QPushButton[rol="principal"] {
	background-color: #899B83;
	color: #FFFFFF;
	border: none;
}

QPushButton[rol="principal"]:hover {
	background-color: #788B72;
}

QPushButton[rol="principal"]:pressed {
	background-color: #6D8067;
}

QPushButton[rol="secundario"] {
	background-color: #FFFFFF;
	color: #899B83;
	border: 1px solid #899B83;
}

QPushButton[rol="secundario"]:hover {
	background-color: #F0F3EE;
	color: #788B72;
}

QPushButton[rol="secundario"]:pressed {
	background-color: #E5EAE2;
	color: #6D8067;
}

QPushButton[rol="peligro"] {
	background-color: transparent;
	color: #899B83;
	border: 1px solid #E8E7E1;
}

QPushButton[rol="peligro"]:hover {
	background-color: #F8EFEC;
	color: #A45D52;
	border-color: #D7B1AA;
}

QPushButton[rol="peligro"]:pressed {
	background-color: #F0DDD8;
	color: #88483F;
	border-color: #C89990;
}

QDoubleSpinBox {
	background-color: #FAFAF8;
	color: #292A28;
	border: 1px solid #E8E7E1;
	border-radius: 8px;
	font-family: "Segoe UI";
	font-size: 13px;
	padding: 0 13px;
	min-height: 46px;
}

QDoubleSpinBox:focus {
	border: 1px solid #899B83;
}

QDoubleSpinBox:disabled {
	color: #92918B;
	background-color: #F5F3EE;
}

QLabel#resultado[estado="exito"] {
	color: #899B83;
	font-family: "Segoe UI";
	font-size: 28px;
	font-weight: 600;
}

QLabel#resultado[estado="error"] {
	color: #92918B;
	font-family: "Segoe UI";
	font-size: 11px;
	font-weight: 600;
	padding: 8px;
}
)");
}
}
