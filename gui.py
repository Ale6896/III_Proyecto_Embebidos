import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QSlider, QVBoxLayout, QWidget
from PyQt5.QtCore import Qt, QTimer
import serial

class MainWindow(QMainWindow):
    def _init_(self):
        super()._init_()
        self.setWindowTitle("Interfaz de Control de Temperatura")
        self.setGeometry(100, 100, 400, 200)
        
        # Configuración de la conexión serial
        self.serial_port = 'COM3'  # Reemplaza 'COMX' con el puerto COM correcto
        self.baud_rate = 9600

        # Crear etiquetas para mostrar la información
        self.label_temp = QLabel("Temperatura: --- °C")
        self.label_umbral = QLabel("Umbral: 20 °C")

        # Crear un control deslizante para ajustar el umbral
        self.slider_umbral = QSlider(Qt.Horizontal)
        self.slider_umbral.setMinimum(0)
        self.slider_umbral.setMaximum(50)
        self.slider_umbral.setValue(20)
        self.slider_umbral.valueChanged.connect(self.update_umbral)

        # Configurar el diseño de la ventana
        layout = QVBoxLayout()
        layout.addWidget(self.label_temp)
        layout.addWidget(self.label_umbral)
        layout.addWidget(self.slider_umbral)

        central_widget = QWidget()
        central_widget.setLayout(layout)
        self.setCentralWidget(central_widget)

        # Configurar la comunicación serial
        self.arduino = serial.Serial(self.serial_port, self.baud_rate)

        # Configurar un temporizador para actualizar los datos del sensor
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_temperatura)
        self.timer.start(1000)  # Actualizar cada segundo

    def update_temperatura(self):
        # Enviar comando para solicitar los datos del sensor al Arduino
        self.arduino.write(b'T')

        # Leer la respuesta del Arduino
        respuesta = self.arduino.readline().decode().strip()

        # Actualizar la etiqueta de temperatura con los datos recibidos
        self.label_temp.setText(f"Temperatura: {respuesta} °C")

    def update_umbral(self, valor):
        # Actualizar la etiqueta de umbral con el valor seleccionado
        self.label_umbral.setText(f"Umbral: {valor} °C")

        # Enviar el nuevo umbral al Arduino
        self.arduino.write(f'U{valor}'.encode())

if _name_ == "_main_":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
