import serial
import keyboard  # Para capturar las teclas presionadas

# Configura el puerto serial (modifica 'COMx' según tu puerto y plataforma)
ser = serial.Serial('COM3', 115200)  # Cambia 'COM3' por el puerto de tu sistema

print("Presiona cualquier tecla para enviarla al ESP32...")

while True:
    if keyboard.is_pressed():  # Detecta si se presiona cualquier tecla
        key = keyboard.read_event(suppress=True).name  # Obtiene la tecla presionada
        print(f"Tecla presionada: {key}")  # Muestra la tecla presionada

        # Envía la tecla al ESP32
        ser.write(key.encode())  # Envía la tecla como bytes

