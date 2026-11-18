from qiskit import QuantumCircuit
from qiskit_aer import AerSimulator

# 1. Definición del circuito: 2 cúbits cuánticos y 2 bits clásicos
qc = QuantumCircuit(2, 2)

# 2. Creación de superposición equiprobable en el cúbit 0
qc.h(0)

# 3. Creación de entrelazamiento máximo mediante CNOT
qc.cx(0, 1)

# 4. Medición de los cúbits en los registros clásicos
qc.measure([0, 1], [0, 1])

# 5. Visualización del circuito en consola
print("CIRCUITO CUÁNTICO:")
print(qc.draw(output="text"))

# 6. Instanciación del simulador y ejecución de 10,000 disparos (shots)
backend = AerSimulator()
shots = 10000
job = backend.run(qc, shots=shots)
counts = job.result().get_counts(qc)

# 7. Presentación de los resultados estadísticos
print(f"\nESPERADO: |00⟩ y |11⟩ cerca del 50 %; |01⟩ y |10⟩ en 0 %.")
print(f"RESULTADOS DE LA MEDICIÓN ({shots} disparos):")
for estado in ("00", "01", "10", "11"):
    count = counts.get(estado, 0)
    porcentaje = (count / shots) * 100
    print(f"Estado |{estado}⟩: {count} ocurrencias ({porcentaje:.2f}%)")

print()
print("Justificación: el Hadamard deja a q0 en superposición (|0⟩+|1⟩)/√2 y la")
print("CNOT copia ese valor en q1. El estado es (|00⟩+|11⟩)/√2. Las ramas 01 y")
print("10 tienen amplitud cero; 00 y 11 salen cada una con probabilidad 1/2.")
